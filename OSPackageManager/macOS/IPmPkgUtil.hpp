#pragma once

#include <vector>
#include <string>
#include <filesystem>

struct PmPackageInfo {
    std::string packageIdentifier;
    std::string version;
    std::string installationPath;
};

class PkgUtilException : public std::runtime_error {
public:
    PkgUtilException(const std::string& message) : std::runtime_error(message) {}
};

class IPmPkgUtil {
public:
    virtual ~IPmPkgUtil() = default;
    
    virtual std::vector<std::string> listPackages(const std::string& volumePath = std::string()) const = 0;
    virtual PmPackageInfo getPackageInfo(const std::string& packageIdentifier, const std::string& volumePath = std::string()) const = 0;
    virtual std::vector<std::string> listPackageFiles(const std::string& packageIdentifier, const std::string& volumePath = std::string()) const = 0;
    virtual bool installPackage(const std::string& packagePath, const std::string& volumePath = std::string()) const = 0;
    virtual bool uninstallPackage(const std::string& packageIdentifier) const = 0;
    virtual bool verifyPackageCodesign(const std::filesystem::path& packagePath, std::string& signer ) const = 0;
};
