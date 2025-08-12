#pragma once

#include "IPackageUtil.hpp"
#include "OSPackageManager/common/ICommandExec.hpp"
#include "PackageManager/IPmPlatformConfiguration.h"

/**
 * @brief A class that implements the 'PackageUtil' utility to perform package-related operations for DEB.
 * Implements the IPackageUtil interface.
 */
class PackageUtilDEB : public IPackageUtil {
public:

    /**
     * @brief Constructor for DEB package operations.
     */
    PackageUtilDEB(ICommandExec &commandExecutor, IPmPlatformConfiguration &platformConfig);

    ~PackageUtilDEB() = default;

    bool isValidInstallerType(const std::string &installerType) const override;
    std::vector<std::string> listPackages() const override;
    PackageInfo getPackageInfo(const PKG_ID_TYPE& identifierType, const std::string& packageIdentifier) const override;
    std::vector<std::string> listPackageFiles(const PKG_ID_TYPE& identifierType, const std::string& packageIdentifier) const override;
    
    /**
     * @brief Installs a package with catalog context information.
     * @param packagePath The path to the package.
     * @param catalogProductAndVersion Catalog information (e.g. "uc/1.0.0.150") from manifest.
     * @param installOptions Options for installation (optional).
     * @return True if the installation was successful, false otherwise.
     */
    bool installPackageWithContext(
        const std::string& packagePath, 
        const std::string& catalogProductAndVersion,
        const std::map<std::string, int>& installOptions = {}) const override;
    
    bool uninstallPackage(const std::string& packageIdentifier) const override;
    bool verifyPackage(const std::string& packagePath, const std::string& signerKeyID) const override;

private:
    ICommandExec &commandExecutor_;
    IPmPlatformConfiguration &platformConfig_;
    
    /**
     * @brief Extracts package info from catalog context (e.g. "uc/1.0.0.150" -> "uc_1.0.0.150").
     * @param catalogProductAndVersion The catalog product and version string from manifest.
     * @return Formatted package name and version for logging.
     */
    std::string extractPackageInfoFromCatalog(const std::string& catalogProductAndVersion) const;
};
