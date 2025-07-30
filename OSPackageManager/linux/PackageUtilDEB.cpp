#include "PackageUtilDEB.hpp"
#include "PmLogger.hpp"
#include "PackageManager/IPmPlatformConfiguration.h"
#include <string.h>
#include <fstream>
#include <algorithm>
#include <ctime>
#include <filesystem>
#include <json/json.h>
#include <sstream>
#include <curl/curl.h>

namespace { //anonymous namespace
    const std::string debPackageInstaller {"deb"};
    const std::string dpkgBinStr {"/bin/dpkg"};
    const std::string dpkgGetPkgInfoOption {"-s"};
    const std::string dpkgListPkgFilesOption {"-L"};
    const std::string dpkgInstallPkgOption {"-i"}; //Supports both install and upgrade
    const std::string dpkgUninstallPkgOption {"-P"}; // -P: Purge (Removes configuration files also), -r: Remove (Keeps configuration files)
    const std::string dpkgSigBinStr {"/bin/dpkg-sig"};
    const std::string dpkgSigVerifyOption {"--verify"};
    const int signer_keyID_pos = 3; // The position of the key ID in the output line of dpkg-sig
    typedef enum {
        SIG_GOOD = 0,
        SIG_BAD = 2,
        SIG_UNKNOWN = 3,
        SIG_NOT_SIGNED = 4
    } SIG_STATUS; // based on the return code of dpkg-sig command
    
    // Callback function for curl to write response data
    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp) {
        userp->append((char*)contents, size * nmemb);
        return size * nmemb;
    }
    
    // Read package name from catalog server endpoint based on installer path or SHA
    std::string getPackageNameFromCatalog(const std::string& packagePath, IPmPlatformConfiguration* platformConfig) {
        if (!platformConfig) {
            PM_LOG_ERROR("Platform configuration is null");
            return "";
        }
        
        // Get catalog URL from platform configuration
        PmUrlList urls;
        if (!platformConfig->GetPmUrls(urls)) {
            PM_LOG_ERROR("Failed to get PM URLs from platform configuration");
            return "";
        }
        
        if (urls.catalogUrl.empty()) {
            PM_LOG_ERROR("Catalog URL is empty in platform configuration");
            return "";
        }
        
        const std::string& catalogUrl = urls.catalogUrl;
        PM_LOG_DEBUG("Using catalog URL from platform config: %s", catalogUrl.c_str());
        
        CURL *curl;
        CURLcode res;
        std::string catalogContent;
        
        curl = curl_easy_init();
        if (!curl) {
            PM_LOG_ERROR("Failed to initialize curl");
            return "";
        }
        
        curl_easy_setopt(curl, CURLOPT_URL, catalogUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &catalogContent);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); // 10 second timeout
        
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        
        if (res != CURLE_OK) {
            PM_LOG_ERROR("Failed to fetch catalog from server: %s", curl_easy_strerror(res));
            return "";
        }
        
        if (catalogContent.empty()) {
            PM_LOG_ERROR("Received empty catalog content from server");
            return "";
        }
        
        Json::Value root;
        Json::CharReaderBuilder builder;
        std::string parseErrors;
        std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        
        if (!reader->parse(catalogContent.c_str(), catalogContent.c_str() + catalogContent.length(), &root, &parseErrors)) {
            PM_LOG_ERROR("Failed to parse catalog JSON: %s", parseErrors.c_str());
            return "";
        }
        
        if (!root.isMember("packages") || !root["packages"].isArray()) {
            PM_LOG_ERROR("Invalid catalog format: missing packages array");
            return "";
        }
        
        // Extract filename from package path for comparison
        std::string packageFilename;
        size_t lastSlash = packagePath.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            packageFilename = packagePath.substr(lastSlash + 1);
        } else {
            packageFilename = packagePath;
        }
        
        // Also extract potential SHA from filename (remove .deb extension)
        std::string potentialSha = packageFilename;
        size_t dotPos = potentialSha.find_last_of(".");
        if (dotPos != std::string::npos) {
            potentialSha = potentialSha.substr(0, dotPos);
        }
        
        // Search through packages in catalog
        const Json::Value& packages = root["packages"];
        for (const Json::Value& package : packages) {
            if (package.isMember("installer_uri") && package.isMember("name")) {
                std::string installerUri = package["installer_uri"].asString();
                
                // Extract filename from installer URI
                size_t uriLastSlash = installerUri.find_last_of("/");
                if (uriLastSlash != std::string::npos) {
                    std::string catalogFilename = installerUri.substr(uriLastSlash + 1);
                    
                    // Match by filename first
                    if (catalogFilename == packageFilename) {
                        std::string catalogName = package["name"].asString();
                        PM_LOG_DEBUG("Found catalog package name by filename match: %s for file: %s", catalogName.c_str(), packageFilename.c_str());
                        return catalogName;
                    }
                }
                
                // If filename didn't match, try matching by SHA256
                if (package.isMember("installer_sha256")) {
                    std::string catalogSha = package["installer_sha256"].asString();
                    if (catalogSha == potentialSha) {
                        std::string catalogName = package["name"].asString();
                        PM_LOG_DEBUG("Found catalog package name by SHA256 match: %s for SHA: %s", catalogName.c_str(), potentialSha.c_str());
                        return catalogName;
                    }
                }
            }
        }
        
        PM_LOG_DEBUG("Package not found in catalog for file: %s (SHA: %s)", packageFilename.c_str(), potentialSha.c_str());
        return "";
    }
    
    // Extract package name and version from DEB file - CATALOG ONLY
    // Try catalog first for consistency, fallback to DEB metadata if catalog unavailable
    std::string extractPackageNameVersion(const std::string& packagePath, ICommandExec& commandExecutor, IPmPlatformConfiguration* platformConfig) {
        PM_LOG_DEBUG("Extracting package name and version from: %s", packagePath.c_str());
        
        std::string packageName;
        
        // FORCE catalog lookup - no fallback to DEB metadata
        if (platformConfig == nullptr) {
            PM_LOG_ERROR("Platform configuration is null - cannot perform catalog lookup");
            return "";
        }
        
        packageName = getPackageNameFromCatalog(packagePath, platformConfig);
        if (packageName.empty()) {
            PM_LOG_ERROR("REQUIRED catalog lookup failed for package: %s - no fallback allowed", packagePath.c_str());
            return "";
        }
        
        PM_LOG_DEBUG("Successfully got package name from catalog: %s", packageName.c_str());
        
        // Query package version from DEB file (still need this for version info)
        std::vector<std::string> versionQueryArgv = {"/usr/bin/dpkg-deb", "-f", packagePath, "Version"};
        int exitCode = 0;
        std::string packageVersion;
        
        int ret = commandExecutor.ExecuteCommandCaptureOutput("/usr/bin/dpkg-deb", versionQueryArgv, exitCode, packageVersion);
        if (ret == 0 && exitCode == 0 && !packageVersion.empty()) {
            // Remove any trailing whitespace/newlines
            packageVersion.erase(packageVersion.find_last_not_of(" \n\r\t") + 1);
            
            std::string result = packageName + "_" + packageVersion;
            PM_LOG_DEBUG("Successfully created package name_version from catalog: %s", result.c_str());
            return result;
        } else {
            PM_LOG_ERROR("Failed to extract version from DEB file: %s", packagePath.c_str());
            return ""; // Return empty string to indicate failure
        }
    }
    
    // Save installer output to log file
    void saveInstallerLog(const std::string& logFilePath, const std::string& output) {
        try {
            // Ensure the directory exists using filesystem API
            std::filesystem::path logPath(logFilePath);
            std::filesystem::path logDir = logPath.parent_path();
            
            if (!std::filesystem::exists(logDir)) {
                std::filesystem::create_directories(logDir);
            }
            
            // Write the log file
            std::ofstream logFile(logFilePath);
            if (logFile.is_open()) {
                // Add a timestamp header
                time_t now = time(nullptr);
                char timeStr[100];
                strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&now));
                
                logFile << "=== Installation Log - " << timeStr << " ===" << std::endl;
                logFile << output << std::endl;
                logFile.close();
                
                // Set proper permissions (644) to match other log files
                std::filesystem::permissions(logFilePath, 
                    std::filesystem::perms::owner_read | std::filesystem::perms::owner_write |
                    std::filesystem::perms::group_read | std::filesystem::perms::others_read);
            } else {
                PM_LOG_ERROR("Failed to create log file: %s", logFilePath.c_str());
            }
        } catch (const std::exception& e) {
            PM_LOG_ERROR("Error creating log directory or file: %s", e.what());
        }
    }

    void retrieveFingerprint(const std::string& outputLine, std::string& fingerprint) {
        std::istringstream stream(outputLine);
        std::string word;
        int wordCnt = 0;

        while (stream >> word) {
            ++wordCnt;
            if (wordCnt == signer_keyID_pos) {
                fingerprint = word;
                break;
            }
        }
    }

    bool matchSignerKeyID(const std::string& fingerprint, const std::string& signerKeyID) {
        std::string keyID {};
        if (fingerprint.length() > 16) {
            keyID = fingerprint.substr(fingerprint.length() - 16);
        } else {
            return false; // Invalid fingerprint to match signerKeyID
        }
        return keyID == signerKeyID;
    }
}

PackageUtilDEB::PackageUtilDEB(ICommandExec &commandExecutor) : commandExecutor_( commandExecutor ), platformConfig_(nullptr) {
}

PackageUtilDEB::PackageUtilDEB(ICommandExec &commandExecutor, IPmPlatformConfiguration* platformConfig) : commandExecutor_( commandExecutor ), platformConfig_(platformConfig) {
    PM_LOG_DEBUG("PackageUtilDEB constructed with platform configuration");
}

void PackageUtilDEB::setPlatformConfiguration(IPmPlatformConfiguration* platformConfig) {
    platformConfig_ = platformConfig;
    PM_LOG_DEBUG("Platform configuration set for PackageUtilDEB");
}

bool PackageUtilDEB::isValidInstallerType(const std::string &installerType) const {
    return installerType == debPackageInstaller;
}

std::vector<std::string> PackageUtilDEB::listPackages() const {
    // Currently not used, hence not implemented. 
    std::vector<std::string>result;
    return result;
}

PackageInfo PackageUtilDEB::getPackageInfo(const PKG_ID_TYPE& identifierType, const std::string& packageIdentifier) const {
    if(identifierType != PKG_ID_TYPE::NAME) {
        PM_LOG_ERROR("Invalid identifier type for value(%s). Currently only pkgname is supported.", packageIdentifier.c_str());
        return {};
    }

    PackageInfo packageInfo;
    std::string packageName {};
    std::string packageVersion {};
    std::string packageArchitecture {};
    std::vector<std::string> infoArgv = {dpkgBinStr, dpkgGetPkgInfoOption, packageIdentifier};
    int exitCode = 0;
    std::string outputBuffer;

    int ret = commandExecutor_.ExecuteCommandCaptureOutput(dpkgBinStr, infoArgv, exitCode, outputBuffer);
    if(ret != 0){
        PM_LOG_ERROR("Failed to execute get package info command.");
    } else if(exitCode != 0) {
        PM_LOG_ERROR("Failed to get package info. Exit code: %d", exitCode);
    } else {
        std::vector<std::string> outputLines;
        commandExecutor_.ParseOutput(outputBuffer, outputLines);

        for (const auto& line : outputLines) {
            if (line.find("Package:") == 0) {
                packageName = line.substr(strlen("Package: "));
            } else if (line.find("Version:") == 0) {
                packageVersion = line.substr(strlen("Version: "));
            } else if (line.find("Architecture:") == 0) {
                packageArchitecture = line.substr(strlen("Architecture: "));
            }
        }

        // Populate the packageInfo with the obtained values.
        packageInfo.packageIdentifier = packageName + "-" + packageVersion + "." + packageArchitecture;
        packageInfo.packageName = packageName;
        packageInfo.version = packageVersion;
    }

    return packageInfo;
}

std::vector<std::string> PackageUtilDEB::listPackageFiles(const PKG_ID_TYPE& identifierType, const std::string& packageIdentifier) const {
    (void) identifierType; // Currently this is of no use as dpkg -L command works with both name and NVRA format similarly.
    std::vector<std::string>result;
    std::vector<std::string> listArgv = {dpkgBinStr, dpkgListPkgFilesOption, packageIdentifier};
    int exitCode = 0;
    std::string outputBuffer;

    int ret = commandExecutor_.ExecuteCommandCaptureOutput(dpkgBinStr, listArgv, exitCode, outputBuffer);
    if(ret != 0){
        PM_LOG_ERROR("Failed to execute list package files command.");
    } else if(exitCode != 0) {
        PM_LOG_ERROR("Failed to list package files. Exit code: %d", exitCode);
    } else {
        commandExecutor_.ParseOutput(outputBuffer, result);
    }  

    return result;
}

bool PackageUtilDEB::installPackage(const std::string& packagePath, const std::map<std::string, int>&  installOptions) const {
    (void) installOptions; // Currently this is of no use.
    
    // Build tracking log to confirm we're using the latest version
    PM_LOG_INFO("[BUILD_TRACK] PackageUtilDEB::installPackage - Version with debug logs and dpkg-deb fixes - 2025-07-05-16:09");
    
    // Extract product name and version from DEB file for logging
    std::string packageNameVersion = extractPackageNameVersion(packagePath, commandExecutor_, platformConfig_);
    std::string logFileName = packageNameVersion;
    std::string logFilePath = "/var/logs/cisco/secureclient/cloudmanagement/" + logFileName + ".log";
    
    PM_LOG_INFO("Installing package %s, logs will be saved to %s", packagePath.c_str(), logFilePath.c_str());

    // Execute installation command (using shell to capture stderr)
    std::string installCmd = std::string(dpkgBinStr) + " " + dpkgInstallPkgOption + " --force-depends --force-confold '" + packagePath + "' 2>&1";
    std::vector<std::string> installArgv = {"/bin/sh", "-c", installCmd};
    int exitCode = 0;
    std::string dpkgOutput;

    PM_LOG_DEBUG("Executing dpkg command with shell: %s", installCmd.c_str());
    int ret = commandExecutor_.ExecuteCommandCaptureOutput("/bin/sh", installArgv, exitCode, dpkgOutput);
    
    PM_LOG_DEBUG("dpkg installation result: ret=%d, exitCode=%d, output length=%zu", ret, exitCode, dpkgOutput.length());
    PM_LOG_DEBUG("dpkg installation output: %s", dpkgOutput.c_str());
    
    // Save the installation output to log file (matching RPM format)
    saveInstallerLog(logFilePath, dpkgOutput);
    
    if(ret != 0){
        PM_LOG_ERROR("Failed to execute install package command. Return code: %d", ret);
        return false;
    } else if(exitCode != 0) {
        PM_LOG_ERROR("Failed to install package. Exit code: %d, Output: %s", exitCode, dpkgOutput.c_str());
        return false;
    }
    
    PM_LOG_INFO("Package installed successfully: %s", packageNameVersion.c_str());
    return true;
}

bool PackageUtilDEB::uninstallPackage(const std::string& packageIdentifier) const {
    std::vector<std::string> uninstallArgv = {dpkgBinStr, dpkgUninstallPkgOption, packageIdentifier};
    int exitCode = 0;

    int ret = commandExecutor_.ExecuteCommand(dpkgBinStr, uninstallArgv, exitCode);
    if(ret != 0){
        PM_LOG_ERROR("Failed to execute uninstall package command.");
        return false;
    } else if(exitCode != 0) {
        PM_LOG_ERROR("Failed to uninstall package. Exit code: %d", exitCode);
        return false;
    } 

    PM_LOG_INFO("Package uninstalled successfully.");
    return true;
}

// NOTE: packagePath is expected to be the complete path to the debian package (e.g., "/home/Downloads/package-1.0.0-1.x86_64.deb")
bool PackageUtilDEB::verifyPackage(const std::string& packagePath, const std::string& signerKeyID) const {;
    
    if (signerKeyID.empty()) {
        PM_LOG_ERROR("Invalid Signer key ID provided.");
        return false;
    }

    int exit_code = 0;
    std::vector<std::string> package_check_argv{ dpkgSigBinStr, dpkgSigVerifyOption, packagePath };
    std::string outputBuffer {};
    std::vector<std::string> outputLines {};
    std::string fingerprint {};

    int ret = commandExecutor_.ExecuteCommandCaptureOutput(dpkgSigBinStr, package_check_argv, exit_code, outputBuffer);
    if(ret != 0){
        PM_LOG_ERROR("Failed to execute verify package command.");
        return false;
    }

    switch (exit_code) {
        case SIG_GOOD:
            commandExecutor_.ParseOutput(outputBuffer, outputLines);
            if(outputLines.empty() || outputLines.size() < 2) {
                PM_LOG_ERROR("Failed to parse output from dpkg-sig.");
                return false;
            }

            retrieveFingerprint(outputLines[outputLines.size()-1], fingerprint);
            if(!matchSignerKeyID(fingerprint, signerKeyID)) {
                PM_LOG_ERROR("Signer key ID mismatch.");
                return false;
            }

            PM_LOG_INFO("Package %s is signed and verified.", packagePath.c_str());
            return true;
        case SIG_BAD:
            PM_LOG_ERROR("Package %s verification failed due to corrupted signature.", packagePath.c_str());
            return false;
        case SIG_UNKNOWN:
            PM_LOG_ERROR("Package %s verification failed due to unknown signature.", packagePath.c_str());
            return false;
        case SIG_NOT_SIGNED:
            PM_LOG_INFO("Package %s is not signed.", packagePath.c_str());
            return false;
        default:
            PM_LOG_ERROR("Package %s verification failed with unknown error.", packagePath.c_str());
            return false;
    }
}