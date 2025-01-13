#pragma once

#include <vector>
#include <string>
#include <map>

struct PackageInfo {
    std::string packageIdentifier;
    std::string packageName;
    std::string version;
};

class PackageUtilException : public std::runtime_error {
public:
    PackageUtilException(const std::string& message) : std::runtime_error(message) {}
};

class IPackageUtil {
public:
    virtual ~IPackageUtil() = default;
    
    virtual std::vector<std::string> listPackages() const = 0;
    virtual PackageInfo getPackageInfo(const std::string& identifierType, const std::string& packageIdentifier) const = 0;
    virtual std::vector<std::string> listPackageFiles(const std::string& identifierType, const std::string& packageIdentifier) const = 0;
    virtual bool installPackage(const std::string& packagePath, const std::map<std::string, int>&  installOptions = {}) const = 0;
    virtual bool uninstallPackage(const std::string& identifierType, const std::string& packageIdentifier) const = 0;
};
