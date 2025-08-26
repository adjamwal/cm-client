#pragma once
#include <stdexcept>
#include <vector>
#include <string>
#include <map>

struct PackageInfo {
    std::string packageIdentifier;
    std::string packageName;
    std::string version;
};
typedef enum
{
    NAME = 0,
    NVRA = 1
    
}PKG_ID_TYPE;

class PkgUtilException : public std::runtime_error {
public:
    PkgUtilException(const std::string& message) : std::runtime_error(message) {}
};

class IPackageUtil {
public:
    virtual ~IPackageUtil() = default;
    
    virtual bool isValidInstallerType(const std::string &installerType) const = 0;
    virtual std::vector<std::string> listPackages() const = 0;
    virtual PackageInfo getPackageInfo(const PKG_ID_TYPE& identifierType, const std::string& packageIdentifier) const = 0;
    virtual std::vector<std::string> listPackageFiles(const PKG_ID_TYPE& identifierType, const std::string& packageIdentifier) const = 0;
    
    // Install with catalog context (catalog information from manifest)
    virtual bool installPackageWithContext(
        const std::string& packagePath, 
        const std::string& catalogProductAndVersion,  // e.g. "uc/1.0.0.150"
        const std::map<std::string, int>& installOptions = {}) const = 0;
    
    virtual bool uninstallPackage(const std::string& packageIdentifier) const = 0;
    virtual bool verifyPackage(const std::string& packagePath, const std::string& signerKeyID) const = 0;
};