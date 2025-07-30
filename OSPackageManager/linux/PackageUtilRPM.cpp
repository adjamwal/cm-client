#include <cstring>
#include <string.h>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <ctime>
#include <filesystem>
#include <json/json.h>
#include <curl/curl.h>

#include "PackageUtilRPM.hpp"
#include "PmLogger.hpp"
#include "PackageManager/IPmPlatformConfiguration.h"
#include "Gpg/include/GpgKeyId.hpp"

#define LONG_KEY_LEN 16

namespace { //anonymous namespace
    const std::string rpmLibPath {"/usr/lib64/librpm.so"};
    const std::string rpmBinStr {"/bin/rpm"};
    const std::string rpmListPkgOption {"-qa"};
    const std::string rpmListPkgFilesOption {"-ql"};
    const std::string rpmInstallPkgOption {"-U"}; //Supports both install and upgrade
    const std::string rpmUninstallPkgOption {"-e"};
    const std::string rpmKeyFormatStr {"%{RSAHEADER:pgpsig}"};
    const std::string rpmNotSignedStr {"(none)"};
    const std::string rpmKeyIdStr {" Key ID "};
    const std::string rpmPubKeySearchStr {"gpg-pubkey"};
    const std::string rpmPubkeyFormatStr {"%{DESCRIPTION}"};
    const std::string rpmPackageInstaller {"rpm"};
    
    // Callback function for curl to write response data
    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp) {
        userp->append((char*)contents, size * nmemb);
        return size * nmemb;
    }
    
    // Read package name from catalog server endpoint based on installer path or SHA
    std::string getPackageNameFromCatalog(const std::string& packagePath, IPmPlatformConfiguration* platformConfig) {
        PM_LOG_DEBUG("getPackageNameFromCatalog: Starting catalog lookup for package: %s", packagePath.c_str());
        
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
        
        PM_LOG_DEBUG("Catalog content length: %zu bytes", catalogContent.length());
        
        Json::Value root;
        Json::CharReaderBuilder builder;
        std::string parseErrors;
        std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        
        if (!reader->parse(catalogContent.c_str(), catalogContent.c_str() + catalogContent.length(), &root, &parseErrors)) {
            PM_LOG_ERROR("Failed to parse catalog JSON: %s", parseErrors.c_str());
            return "";
        }
        
        PM_LOG_DEBUG("Catalog JSON parsed successfully");
        
        if (!root.isMember("packages") || !root["packages"].isArray()) {
            PM_LOG_ERROR("Invalid catalog format: missing packages array");
            return "";
        }
        
        const Json::Value& packages = root["packages"];
        PM_LOG_DEBUG("Found %u packages in catalog", packages.size());
        
        // Extract filename from package path for comparison
        std::string packageFilename;
        size_t lastSlash = packagePath.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            packageFilename = packagePath.substr(lastSlash + 1);
        } else {
            packageFilename = packagePath;
        }
        
        // Also extract potential SHA from filename (remove .rpm extension)
        std::string potentialSha = packageFilename;
        size_t dotPos = potentialSha.find_last_of(".");
        if (dotPos != std::string::npos) {
            potentialSha = potentialSha.substr(0, dotPos);
        }
        
        PM_LOG_DEBUG("Searching catalog for: filename='%s', SHA='%s'", packageFilename.c_str(), potentialSha.c_str());
        
        // Search through packages in catalog
        for (const Json::Value& package : packages) {
            if (package.isMember("installer_uri") && package.isMember("name")) {
                std::string installerUri = package["installer_uri"].asString();
                std::string packageName = package["name"].asString();
                
                PM_LOG_DEBUG("Checking catalog package: name='%s', uri='%s'", packageName.c_str(), installerUri.c_str());
                
                // Extract filename from installer URI
                size_t uriLastSlash = installerUri.find_last_of("/");
                if (uriLastSlash != std::string::npos) {
                    std::string catalogFilename = installerUri.substr(uriLastSlash + 1);
                    
                    PM_LOG_DEBUG("  Catalog filename: '%s'", catalogFilename.c_str());
                    
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
                    
                    PM_LOG_DEBUG("  Catalog SHA256: '%s'", catalogSha.c_str());
                    
                    if (catalogSha == potentialSha) {
                        std::string catalogName = package["name"].asString();
                        PM_LOG_DEBUG("Found catalog package name by SHA256 match: %s for SHA: %s", catalogName.c_str(), potentialSha.c_str());
                        return catalogName;
                    }
                }
            } else {
                PM_LOG_DEBUG("Skipping package entry - missing installer_uri or name field");
            }
        }
        
        PM_LOG_DEBUG("Package not found in catalog for file: %s (SHA: %s)", packageFilename.c_str(), potentialSha.c_str());
        return "";
    }
    
    // Extract package name and version from RPM file - CATALOG ONLY
    // Always read package name from catalog server to ensure consistency
    std::string extractPackageNameVersion(const std::string& packagePath, ICommandExec& commandExecutor, IPmPlatformConfiguration* platformConfig) {
        std::string packageName;
        
        // FORCE catalog lookup - no fallback to RPM metadata
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
        
        // Query package version from RPM file (still need this for version info)
        std::vector<std::string> versionQueryArgv = {rpmBinStr, "-qp", "--queryformat", "%{VERSION}", packagePath};
        int exitCode = 0;
        std::string packageVersion;
        
        int ret = commandExecutor.ExecuteCommandCaptureOutput(rpmBinStr, versionQueryArgv, exitCode, packageVersion);
        if (ret == 0 && exitCode == 0 && !packageVersion.empty()) {
            std::string result = packageName + "_" + packageVersion;
            PM_LOG_DEBUG("Successfully created package name_version from catalog: %s", result.c_str());
            return result;
        } else {
            PM_LOG_ERROR("Failed to extract version from RPM file: %s", packagePath.c_str());
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
}

PackageUtilRPM::PackageUtilRPM(ICommandExec &commandExecutor, IGpgUtil &gpgUtil) : commandExecutor_( commandExecutor ), gpgUtil_( gpgUtil ), platformConfig_(nullptr)  {
    if (!loadLibRPM()) {
        throw PkgUtilException("Failed to load librpm for RPM package operations.");
    }
}

PackageUtilRPM::PackageUtilRPM(ICommandExec &commandExecutor, IGpgUtil &gpgUtil, IPmPlatformConfiguration* platformConfig) : commandExecutor_( commandExecutor ), gpgUtil_( gpgUtil ), platformConfig_(platformConfig)  {
    PM_LOG_DEBUG("PackageUtilRPM constructed with platform configuration");
    if( !loadLibRPM() )
        PM_LOG_ERROR("PackageUtilRPM failed to load librpm");
}

void PackageUtilRPM::setPlatformConfiguration(IPmPlatformConfiguration* platformConfig) {
    platformConfig_ = platformConfig;
    PM_LOG_DEBUG("Platform configuration set for PackageUtilRPM");
}

bool PackageUtilRPM::loadLibRPM() {

    libRPMhandle_ = dlopen(rpmLibPath.c_str(), RTLD_NOW);
    if (!libRPMhandle_) {
        PM_LOG_ERROR("Failed to load librpm: %s", dlerror());
        return false;
    }

    fpRpmReadConfigFiles_ = reinterpret_cast<fpRpmReadConfigFiles_t>(dlsym(libRPMhandle_, "rpmReadConfigFiles"));
    fpRpmTsCreate_ = reinterpret_cast<fpRpmTsCreate_t>(dlsym(libRPMhandle_, "rpmtsCreate"));
    fpRpmReadPackageFile_ = reinterpret_cast<fpRpmReadPackageFile_t>(dlsym(libRPMhandle_, "rpmReadPackageFile"));
    fpRpmTsFree_ = reinterpret_cast<fpRpmTsFree_t>(dlsym(libRPMhandle_, "rpmtsFree"));
    fpRpmTsInitIterator_ = reinterpret_cast<fpRpmTsInitIterator_t>(dlsym(libRPMhandle_, "rpmtsInitIterator"));
    fpRpmDbNextIterator_ = reinterpret_cast<fpRpmDbNextIterator_t>(dlsym(libRPMhandle_, "rpmdbNextIterator"));
    fpRpmDbFreeIterator_ = reinterpret_cast<fpRpmDbFreeIterator_t>(dlsym(libRPMhandle_, "rpmdbFreeIterator"));
    fpHeaderGetString_ = reinterpret_cast<fpHeaderGetString_t>(dlsym(libRPMhandle_, "headerGetString"));

    if (!fpRpmReadConfigFiles_ || !fpRpmTsCreate_ || !fpRpmReadPackageFile_ || !fpRpmTsFree_ || 
        !fpRpmTsInitIterator_ || !fpRpmDbNextIterator_ || 
        !fpRpmDbFreeIterator_ || !fpHeaderGetString_ ) {
        PM_LOG_ERROR("Failed to resolve symbols: %s", dlerror());
        unloadLibRPM();
        return false;
    }

    fpRpmReadConfigFiles_(NULL, NULL);

    return true;
}

bool PackageUtilRPM::unloadLibRPM() {
    int retVal = dlclose(libRPMhandle_);
    if(0 != retVal) {
        PM_LOG_ERROR("Failed to unload librpm: %s", dlerror());
        return false;
    }

    libRPMhandle_ = nullptr;
    return true;
}

bool PackageUtilRPM::isValidInstallerType(const std::string &installerType) const {
    return installerType == rpmPackageInstaller;
}

std::vector<std::string> PackageUtilRPM::listPackages() const {
    std::vector<std::string>result;
    std::vector<std::string> listArgv = {rpmBinStr, rpmListPkgOption};
    int exitCode = 0;
    std::string outputBuffer;

    // To-Do: Make sure the rpmBinStr exists before executing the command.
    int ret = commandExecutor_.ExecuteCommandCaptureOutput(rpmBinStr, listArgv, exitCode, outputBuffer);
    if(ret != 0){
        PM_LOG_ERROR("Failed to execute list packages command.");
    } else if(exitCode != 0) {
        PM_LOG_ERROR("Failed to list packages. Exit code: %d", exitCode);
    } else {
        commandExecutor_.ParseOutput(outputBuffer, result);
    }

    return result;
}

PackageInfo PackageUtilRPM::getPackageInfo(const PKG_ID_TYPE& identifierType, const std::string& packageIdentifier) const {
    // NOTE: This API assumes the caller is sure that the package exists on the system and is asking for the information.
    //       If the package does not exist, the API will return an empty PackageInfo object.

    PackageInfo result;
    (void) identifierType;
    (void) packageIdentifier;

    rpmts ts = fpRpmTsCreate_();
    if(NULL == ts) {
        PM_LOG_ERROR("Failed to create rpm transaction set.");
        return result;
    }

    rpmdbMatchIterator mi = fpRpmTsInitIterator_(ts, RPMDBI_PACKAGES, NULL, 0);

    Header packageHeader;
    while ((packageHeader = fpRpmDbNextIterator_(mi)) != NULL) {
        const char* packageName = fpHeaderGetString_(packageHeader, RPMTAG_NAME);
        const char* packageVersion = fpHeaderGetString_(packageHeader, RPMTAG_VERSION);
        const char* packageRelease = fpHeaderGetString_(packageHeader, RPMTAG_RELEASE);
        const char* packageArch = fpHeaderGetString_(packageHeader, RPMTAG_ARCH);
        // NOTE: We are not taking epoch into consideration for now because it will be mostly 0 for the packages we are interested in.
        //       If later there arises a need to consider epoch, we can add RPMTAG_EPOCH to the headerGetString_ptr function and use it here.
        
        if(NULL == packageName || NULL == packageVersion || NULL == packageRelease || NULL == packageArch) {
            continue;
        }

        std::string packageNVRAFormat = std::string(packageName) + \
                                        "-" + std::string(packageVersion) + \
                                        "-" + std::string(packageRelease) + \
                                        "." + std::string(packageArch);

        if((identifierType == PKG_ID_TYPE::NAME && packageName == packageIdentifier) || \
            (identifierType == PKG_ID_TYPE::NVRA && packageNVRAFormat == packageIdentifier)) {
            result.packageIdentifier = packageNVRAFormat;
            result.packageName = packageName;
            result.version = packageVersion;
            break;
        }
    }

    fpRpmDbFreeIterator_(mi);
    fpRpmTsFree_(ts);

    return result;
}

std::vector<std::string> PackageUtilRPM::listPackageFiles(const PKG_ID_TYPE& identifierType, const std::string& packageIdentifier) const {
    (void) identifierType; // Currently this is of no use as rpm -ql command works with both name and NVRA format similarly.
    std::vector<std::string>result;
    std::vector<std::string> listArgv = {rpmBinStr, rpmListPkgFilesOption, packageIdentifier};
    int exitCode = 0;
    std::string outputBuffer;

    int ret = commandExecutor_.ExecuteCommandCaptureOutput(rpmBinStr, listArgv, exitCode, outputBuffer);
    if(ret != 0){
        PM_LOG_ERROR("Failed to execute list package files command.");
    } else if(exitCode != 0) {
        PM_LOG_ERROR("Failed to list package files. Exit code: %d", exitCode);
    } else {
        commandExecutor_.ParseOutput(outputBuffer, result);
    }  

    return result;
}

bool PackageUtilRPM::installPackage(const std::string& packagePath, const std::map<std::string, int>&  installOptions) const {
    (void) installOptions; // Currently this is of no use.
    
    // Extract product name and version from RPM file for logging
    std::string packageNameVersion = extractPackageNameVersion(packagePath, commandExecutor_, platformConfig_);
    std::string logFileName = packageNameVersion;
    std::string logFilePath = "/var/logs/cisco/secureclient/cloudmanagement/" + logFileName + ".log";
    
    PM_LOG_INFO("Installing package %s, logs will be saved to %s", packagePath.c_str(), logFilePath.c_str());

    // Execute installation command
    std::vector<std::string> installArgv = {rpmBinStr, rpmInstallPkgOption, "--verbose", packagePath};
    int exitCode = 0;
    std::string rpmOutput;

    int ret = commandExecutor_.ExecuteCommandCaptureOutput(rpmBinStr, installArgv, exitCode, rpmOutput);
    
    // Always save the RPM output to log file (success or failure)
    saveInstallerLog(logFilePath, rpmOutput);
    
    if(ret != 0){
        PM_LOG_ERROR("Failed to execute install package command.");
        return false;
    } else if(exitCode != 0) {
        PM_LOG_ERROR("Failed to install package. Exit code: %d", exitCode);
        return false;
    }
    
    PM_LOG_INFO("Package installed successfully: %s", packageNameVersion.c_str());
    return true;
}

bool PackageUtilRPM::uninstallPackage(const std::string& packageIdentifier) const {
    std::vector<std::string> uninstallArgv = {rpmBinStr, rpmUninstallPkgOption, packageIdentifier};
    int exitCode = 0;

    int ret = commandExecutor_.ExecuteCommand(rpmBinStr, uninstallArgv, exitCode);
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

bool PackageUtilRPM::is_trusted_by_system(std::string keyId) const {
    std::vector<std::string> imported_rpm_pubkeys_argv = { rpmBinStr, "-q", rpmPubKeySearchStr };
    std::vector<std::string> pubkey_block_argv = { rpmBinStr, "-q", "--queryformat", rpmPubkeyFormatStr, "" };
    const int current_pubkey_index = 4;
    std::vector<std::string> fingerprint_block_argv = { "/bin/gpg", "--fingerprint", keyId };
    std::string gpg_fingerprint = "";
    int exitCode = 0;

    std::string rpm_pubkeys{ "" };
    if (commandExecutor_.ExecuteCommandCaptureOutput(imported_rpm_pubkeys_argv[0],
                                          imported_rpm_pubkeys_argv,
                                          exitCode,
                                          rpm_pubkeys) ||
        (exitCode != 0)) {
        return false;
    }
    std::istringstream rpm_pubkeys_stream;
    rpm_pubkeys_stream.str(rpm_pubkeys);

    for (std::string line; std::getline(rpm_pubkeys_stream, line);) {
        
        std::string pubkey_block = "";
        pubkey_block_argv[current_pubkey_index] = line;
        if (commandExecutor_.ExecuteCommandCaptureOutput(pubkey_block_argv[0],
                                              pubkey_block_argv,
                                              exitCode,
                                              pubkey_block) ||
            (exitCode != 0)) {
            return false;
        }
        
        std::vector<char> pubkey_block_vector(pubkey_block.begin(), pubkey_block.end());
        GpgKeyId trusted_keyid = gpgUtil_.get_pubkey_fingerprint(pubkey_block_vector);
        if (trusted_keyid.long_id() == keyId) {
            return true;
        }
    }

    if (commandExecutor_.ExecuteCommandCaptureOutput(fingerprint_block_argv[0],
                                              fingerprint_block_argv,
                                              exitCode,
                                              gpg_fingerprint) ||
        (exitCode != 0)) {
        return false;
    }

    if (gpg_fingerprint != "gpg: error reading key: No public key") {
        return true;
    }
    return false;
}

static std::string _rpm_get_keyid(std::string pgpsig)
{
    const char *pgpsig_keyid = NULL;

    /* Parse " Key ID <key id>" */
    pgpsig_keyid = std::strstr(pgpsig.c_str(), rpmKeyIdStr.c_str());
    if (!pgpsig_keyid) {
        return "";
    }

    /* Parse "<key id>" */
    pgpsig_keyid += rpmKeyIdStr.length();
    if (*pgpsig_keyid == '\0') {
        return "";
    }

    return std::string(pgpsig_keyid);
}

bool PackageUtilRPM::verifyPackage(const std::string& packagePath, const std::string& signerKeyID) const {
    
    int exit_code;
    std::string keyId;
    std::string pgpkey;

    std::vector<std::string> package_check_argv{ rpmBinStr, "-q", "--queryformat", rpmKeyFormatStr, "-p", packagePath };

    if (commandExecutor_.ExecuteCommandCaptureOutput(
            rpmBinStr, package_check_argv, exit_code, pgpkey) ||
        (exit_code != 0)) {
        PM_LOG_ERROR("Query package %s for pgpkey failed (%d)", packagePath.c_str(), exit_code);
        return false;
    } else if (rpmNotSignedStr == pgpkey) {
        PM_LOG_ERROR("No key available to validate RPM package - verification failed %s",
            packagePath.c_str());
        return false;
    }

    keyId = _rpm_get_keyid(pgpkey);

    if (keyId.empty()) {
        PM_LOG_INFO("RPM package is not signed: %s", packagePath.c_str());
        return false;
    }

    if (is_trusted_by_system(keyId) && signerKeyID == keyId) {
        return true;
    }
    PM_LOG_INFO("RPM package failed trusted key check: %s", packagePath.c_str());
    return false;
}