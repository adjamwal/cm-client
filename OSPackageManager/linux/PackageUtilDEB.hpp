#pragma once

#include "IPackageUtil.hpp"

/**
 * @brief A class that implements the 'PackageUtil' utility to perform package-related operations for DEB.
 * Implements the IPackageUtil interface.
 */
class PackageUtilDEB : public IPackageUtil {
public:

    PackageUtilDEB() = default;

    ~PackageUtilDEB() = default;

    bool isValidInstallerType(const std::string &installerType) const override;
    std::vector<std::string> listPackages() const override;
    PackageInfo getPackageInfo(const PKG_ID_TYPE& identifierType, const std::string& packageIdentifier) const override;
    std::vector<std::string> listPackageFiles(const PKG_ID_TYPE& identifierType, const std::string& packageIdentifier) const override;
    bool installPackage(const std::string& packagePath, const std::map<std::string, int>&  installOptions = {}) const override;
    bool uninstallPackage(const std::string& packageIdentifier) const override;
};
