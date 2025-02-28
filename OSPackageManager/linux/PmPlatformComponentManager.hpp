/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */
#pragma once

#include "PackageManager/IPmPlatformComponentManager.h"
#include "IPackageUtil.hpp"
#include "PmPlatformDiscovery.hpp"
#include "IFileUtilities.hpp"
#include <memory>

class IPmCodesignVerifier;

class PmPlatformComponentManager : public IPmPlatformComponentManager
{
public:
    PmPlatformComponentManager(std::shared_ptr<IPackageUtil>, std::shared_ptr<PackageManager::IFileUtilities> fileUtils);

    /**
     * @brief This API is used to retrieve the list of all installed packages on the client. The package manager
     *   will provide a list of attributes to lookup for each package
     *
     * @return 0 if the packages have been successfully retrieved. -1 otherwise
     */
    int32_t GetInstalledPackages(const std::vector<PmProductDiscoveryRules> &catalogRules, PackageInventory &packagesDiscovered);

    /**
     * @brief This API is used to retrieve the cached list of packages installed on the client, as detected during the last discovery operation.
     *
     * @return 0 if the packages have been successfully retrieved. -1 otherwise
     */
    int32_t GetCachedInventory(PackageInventory &cachedInventory);

    /**
     * @brief This API will be used to install a package. The package will provide the following:
     *   - Installation binary
     *   - Installation location
     *   - Installation command line
     *   This API should rollback the installation if the package could not be installed
     *
     * @param[in] package - The package details
     * @return 0 if the package was installed. -1 otherwise
     */
    int32_t InstallComponent(const PmComponent &package);

    /**
     * @brief This API will be used to update a package. The package will provide the following:
     *   - Installation binary
     *   - Installation location
     *   - Installation command line
     *   This API should rollback to the previous version if the package could not be updated. The package should be
     *   left in a good working state
     *
     * @param[in] package - The package details
     * @return 0 if the package was updated. -1 otherwise
     */
    PmInstallResult UpdateComponent(const PmComponent &package, std::string &error);

    /**
     * @brief This API will be used to remove a package. The package will provide the following:
     *   - Un-installation binary
     *   - Un-installation command line
     *   This API should rollback to the uninstallation if the package could not be removed. The package should be
     *   left in a good working state
     *
     * @param[in] package - The package details
     * @return 0 if the package was removed. -1 otherwise
     */
    int32_t UninstallComponent(const PmComponent &package);

    /**
     * @brief This API will be used to deploy a configuration file for a package. The configuration will provide the
     *   following:
     *   - Configuration file path
     *   - Configuration contents
     *   - commandline to validate the configuration file
     *
     * @return 0 if the configuration was deployed. -1 otherwise
     */
    int32_t DeployConfiguration(const PackageConfigInfo &config);

    /**
     * @brief This API will be used to resolve a platform specific path
     *   The config path could contain platform specific content ( Windows KNOWN_FOLDER_ID )
     *
     * @return string contaning the resolved
     */
    std::string ResolvePath(const std::string &basePath);


    /**
    * Searches an absolute path for all files or configurables that match wildcard searches
    * Returns a list of all matching absolute paths of files found
    *
    * star is 0 or many
    * question mark is exactly one
    *
    */
    int32_t FileSearchWithWildCard(const std::filesystem::path &searchPath, std::vector<std::filesystem::path> &results);

    /**
    * Initiates a system restart
    *
    */
    void NotifySystemRestart();

    /**
    * @brief (Optional)Provides Generic Read Access to file
    *   Used on windows to give read permissions to the Builtin users groups. Similar to chmod +r
    *
    * @return 0 on success
    */
    int32_t ApplyBultinUsersReadPermissions(const std::filesystem::path &filePath);

    /**
    * @brief Provides Full Access for Admins and System to file/folder, and read-only access to authenticated users
    *   Used on windows to give full permissions to administrators and system users. Similar to chmod +rw
    *
    * @return 0 on success
    */
    int32_t RestrictPathPermissionsToAdmins(const std::filesystem::path &filePath);
    
private:
    std::shared_ptr<IPackageUtil> pkgUtil_;
    PmPlatformDiscovery discovery_;
    std::shared_ptr<PackageManager::IFileUtilities> fileUtils_;
};
