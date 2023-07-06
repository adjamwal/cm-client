#include <iostream>
#include <array>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include "PmPkgUtilWrapper.hpp"

const std::string pkgUtilExecutable{ "/usr/sbin/pkgutil" };

std::vector<std::string> PmPkgUtilWrapper::listPackages(const std::string& volumePath) const {
    std::vector<std::string> packages;
    const std::string command { pkgUtilExecutable + " --packages" };
    std::string output = executeCommand(command, volumePath);

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
    const std::string command { pkgUtilExecutable + " --pkg-info "  + packageIdentifier };
    const std::string output = executeCommand(command, volumePath);
    return parsePackageInfo(output);
}

std::vector<std::string> PmPkgUtilWrapper::listPackageFiles(const std::string& packageIdentifier, const std::string& volumePath) const {
    std::vector<std::string> files;
    const std::string command{ pkgUtilExecutable + " --files " + packageIdentifier };
    std::string output = executeCommand(command, volumePath);

    size_t pos = 0;
    std::string delimiter = "\n";
    while ((pos = output.find(delimiter)) != std::string::npos) {
        std::string file = output.substr(0, pos);
        files.push_back(file);
        output.erase(0, pos + delimiter.length());
    }

    return files;
}

std::string PmPkgUtilWrapper::executeCommand(const std::string& command, const std::string& volumePath) const {
    
    std::string fullCommand = command;
    if (!volumePath.empty()) {
        fullCommand += " --volume " + volumePath;
    }
    
    constexpr size_t BUFSIZE = 128;
    std::array<char, BUFSIZE> buffer;
    std::string result;

    std::shared_ptr<FILE> pipe(popen(fullCommand.c_str(), "r"), pclose);
    if (!pipe) {
        
        throw PkgUtilException("Error executing command `" + fullCommand + "`");
    }

    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
    }

    return result;
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


