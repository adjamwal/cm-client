#include <iostream>
#include <array>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <regex>
#include <filesystem>

#include "PmLogger.hpp"
#include "PmPkgUtilWrapper.hpp"
#include "FileUtilities.hpp"

namespace { // anonymous namespace
    
    const std::string pkgUtilExecutable{ "/usr/sbin/pkgutil" };
    const std::string pkgInstallerExecutable{ "/usr/sbin/installer" };
    const std::string pkgCodesignVerifierExecutable{ "/usr/sbin/spctl" };
    
    const std::regex kSpctlSignerIdPattern("origin=\\* \\(([A-Z0-9]+)\\)");
    const std::regex kPkgUtilSignerIdpattern("Certificate Chain:\n.*\\((\\w+)\\)");
    const std::regex kPkgUtilSignerNamePattern("Developer ID Installer: (\\w+) \\(\\w+\\)");

    const std::vector<std::string> installResultsSuccess = {"The install was successful", "The upgrade was successful"};
    
    bool outputParser(const std::string& output, const std::string& textToFind) {
        return std::regex_search(output, std::regex{textToFind});
    };

    bool outputParser(const std::string& output, const std::vector<std::string>& texts) {
        for (const auto &text : texts) {
            if (outputParser(output, text))
                return  true;
        }
        return false;
    };

    bool extractSignerByPattern(const std::string& input, const std::regex& pattern, std::string& signer) {
        std::smatch matches;
        if (std::regex_search(input, matches, pattern)) {
            if (matches.size() > 1) {
                signer = matches[1].str();
                return true;
            }
        }
        
        return false;
    }

    std::string decoratePkgUtilVolumeOption(const std::string& command, const std::string& volumePath) {
        return volumePath.length() ?
        command + " --volume " + volumePath :
        command;
    }
    
    std::string decoratePkgInstallerVolumeOption(const std::string& command, const std::string& volumePath) {
        const std::string target{ volumePath.length() ? volumePath : "/" };
        return command + " -target " + target;
    }
    
    void trim(std::string& str) {
        // Remove leading and trailing whitespaces from a string
        str.erase(0, str.find_first_not_of(" \t\r\n"));
        str.erase(str.find_last_not_of(" \t\r\n") + 1);
    }
    
    std::vector<std::string> splitString(const std::string_view& str, const char delimiter) {
        std::vector<std::string> tokens;
        size_t startPos = 0;
        while (startPos < str.length()) {
            size_t endPos = str.find(delimiter, startPos);
            if (endPos == std::string::npos) {
                endPos = str.length();
            }
            std::string token = std::string(str.substr(startPos, endPos - startPos));
            if (!token.empty()) {
                tokens.push_back(std::move(token));
            }
            startPos = endPos + 1;
        }
        return tokens;
    }
}

std::vector<std::string> PmPkgUtilWrapper::listPackages(const std::string& volumePath) const {
    PM_LOG_INFO("Listing packages");
    const std::string command{ pkgUtilExecutable + " --packages" };
    std::string output = executeCommand( decoratePkgUtilVolumeOption(command, volumePath) );
    
    std::vector<std::string> packages;
    size_t pos = 0;
    std::string delimiter = "\n";
    while ((pos = output.find(delimiter)) != std::string::npos) {
        std::string package = output.substr(0, pos);
        packages.push_back(package);
        output.erase(0, pos + delimiter.length());
    }
    
    return packages;
}

PmPackageInfo PmPkgUtilWrapper::getPackageInfo(const std::string& packageIdentifier, const std::string& volumePath) const {
    PM_LOG_INFO("Getting package %s info", packageIdentifier.c_str());
    const std::string command { pkgUtilExecutable + " --pkg-info "  + packageIdentifier };
    const std::string output = executeCommand( decoratePkgUtilVolumeOption(command, volumePath) );
    return parsePackageInfo(output);
}

std::vector<std::string> PmPkgUtilWrapper::listPackageFiles(const std::string& packageIdentifier, const std::string& volumePath) const {
    PM_LOG_INFO("List package %s files", packageIdentifier.c_str());
    const std::string command{ pkgUtilExecutable + " --files " + packageIdentifier };
    std::string output = executeCommand( decoratePkgUtilVolumeOption(command, volumePath) );

    std::vector<std::string> files;

    size_t pos = 0;
    std::string delimiter = "\n";
    while ((pos = output.find(delimiter)) != std::string::npos) {
        std::string file = output.substr(0, pos);
        files.push_back(file);
        output.erase(0, pos + delimiter.length());
    }

    return files;
}

std::string PmPkgUtilWrapper::executeCommand(const std::string& command) const {
    
    std::shared_ptr<FILE> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) {
        throw PkgUtilException("Error executing command `" + command + "`");
    }
    
    constexpr size_t BUFSIZE = 128;
    std::array<char, BUFSIZE> buffer;
    std::string result;

    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
    }

    return result;
}

PmPackageInfo PmPkgUtilWrapper::parsePackageInfo(const std::string& output) const {
    
    const std::unordered_map<std::string, std::string PmPackageInfo::*> keyToMember = {
        { "package-id", &PmPackageInfo::packageIdentifier },
        { "version", &PmPackageInfo::version },
        { "location", &PmPackageInfo::installationPath }
    };
    
    PmPackageInfo packageInfo;

    for (const auto& line : splitString(output, '\n')) {
        const size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            auto [key, value] = std::make_tuple(line.substr(0, colonPos), line.substr(colonPos + 1));
            trim(key);
            trim(value);
            
            const auto it = keyToMember.find(key);
            if (it != keyToMember.end()) {
                const auto memberPtr = it->second;
                packageInfo.*memberPtr = value;
            }
        }
    }
    
    return packageInfo;
}

bool PmPkgUtilWrapper::installPackage(const std::string& packagePath, const std::map<std::string, int>&  installOptions, const std::string& volumePath) const {
    auto extractXmlPath = [&](const std::string& packagePath){
        std::string sPath;
        if (packagePath.empty())
            return sPath;

        PackageManager::FileUtilities fu;
        sPath = fu.GenerateTemporaryFileName();
        
        const auto cmdRes = executeCommand(pkgInstallerExecutable + " -pkg " + packagePath + " -showChoiceChangesXML > " + sPath);
        if (!cmdRes.empty())
            sPath.clear();

        return sPath;
    };
    
    const std::string sXmlPath = !installOptions.empty()
                                    ? extractXmlPath(packagePath)
                                    : "";
    
    const std::string applyCommand = !installOptions.empty() && !sXmlPath.empty() ?
                                    " -applyChoiceChangesXML " + sXmlPath
                                    : "";
    if (!installOptions.empty() && !sXmlPath.empty()){
        PackageManager::modifyXmlValues(sXmlPath, installOptions);
    }
    
    PM_LOG_INFO("Installing package %s", packagePath.c_str());
    const std::string command{ pkgInstallerExecutable + " -pkg " + packagePath + applyCommand };
    
    PM_LOG_INFO("Package install command: %s", command.c_str());
    const std::string output = executeCommand( decoratePkgInstallerVolumeOption(command, volumePath));

    if (!installOptions.empty() && std::filesystem::exists(sXmlPath)) {
        std::error_code errCode;
        std::filesystem::remove(sXmlPath, errCode);
    }

    if (outputParser(output, installResultsSuccess)) {
        // Installation successful
        return true;
    } else {
        // Installation failed
        return false;
    }
}

bool PmPkgUtilWrapper::uninstallPackage(const std::string& packageIdentifier) const {
    PM_LOG_INFO("Uninstalling package %s", packageIdentifier.c_str());
    const std::string command{ pkgUtilExecutable + " --force --forget " + packageIdentifier };
    const std::string output = executeCommand(command);
    
    if (outputParser(output, "No receipt")) {
        // Uninstallation successful
        return true;
    } else {
        // Uninstallation failed
        return false;
    }
}

bool PmPkgUtilWrapper::verifyPackageCodesign(const std::filesystem::path& packagePath, std::string& signer) const {
    PM_LOG_INFO("Verify package %s code signing", packagePath.c_str());
//    const std::string command{ pkgCodesignVerifierExecutable + " -a -vvv -t install " + packagePath.string() };
    const std::string command{ pkgUtilExecutable + " --check-signature " + packagePath.string() };
    const std::string output = executeCommand(command);
    
    if (outputParser(output, "signed by") && extractSignerByPattern(output, kPkgUtilSignerNamePattern, signer)) {
        PM_LOG_INFO("Package %s has valid codesigning by %s", packagePath.string().c_str(), signer.c_str());
        return true;
    } else {
        PM_LOG_WARNING("Package %s has no valid codesigning", packagePath.string().c_str());
        return false;
    }
}
