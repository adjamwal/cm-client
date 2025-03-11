#include "PackageUtilDEB.hpp"

bool PackageUtilDEB::isValidInstallerType(const std::string &installerType) const {
    (void) installerType;
    return true;
}

std::vector<std::string> PackageUtilDEB::listPackages() const {
    std::vector<std::string>result;
    return result;
}

PackageInfo PackageUtilDEB::getPackageInfo(const PKG_ID_TYPE& identifierType, const std::string& packageIdentifier) const {
    PackageInfo result;
    (void) identifierType;
    (void) packageIdentifier;
    return result;
}

std::vector<std::string> PackageUtilDEB::listPackageFiles(const PKG_ID_TYPE& identifierType, const std::string& packageIdentifier) const {
    (void) identifierType;
    (void) packageIdentifier;
    std::vector<std::string>result;
    return result;
}

bool PackageUtilDEB::installPackage(const std::string& packagePath, const std::map<std::string, int>&  installOptions) const {
    (void) installOptions;
    (void) packagePath;
    return true;
}

bool PackageUtilDEB::uninstallPackage(const std::string& packageIdentifier) const {
    (void) packageIdentifier;
    return true;
}

bool PackageUtilDEB::verifyPackage(const std::string& packageIdentifier) const {
    (void) packageIdentifier;
    return true;
}