/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "PmPlatformComponentManager.hpp"

int32_t PmPlatformComponentManager::GetInstalledPackages(const std::vector<PmProductDiscoveryRules> &catalogRules, PackageInventory &packagesDiscovered)
{
    (void) catalogRules;
    (void) packagesDiscovered;
    /** @todo how to determine installed packages? */
    /* pkg installers have a package receipt, but what if the package receipt is not
     * removed by an uninstaller?  Then an uninstalled package may be identified as installed.
     */
    return -1;
}

int32_t PmPlatformComponentManager::GetCachedInventory(PackageInventory &cachedInventory)
{
    (void) cachedInventory;
    return -1;
}

int32_t PmPlatformComponentManager::InstallComponent(const PmComponent &package)
{
    (void) package;
    return -1;
}

IPmPlatformComponentManager::PmInstallResult PmPlatformComponentManager::UpdateComponent(const PmComponent &package, std::string &error)
{
    IPmPlatformComponentManager::PmInstallResult result = {
        .pmResult = IPmPlatformComponentManager::PM_INSTALL_FAILURE,
        .platformResult = 0
    };

    (void) package;
    (void) error;
    return result;
}

int32_t PmPlatformComponentManager::UninstallComponent(const PmComponent &package)
{
    (void) package;
    return -1;
}

int32_t PmPlatformComponentManager::DeployConfiguration(const PackageConfigInfo &config)
{
    (void) config;
    return -1;
}

std::string PmPlatformComponentManager::ResolvePath(const std::string &basePath)
{
    return basePath;
}

int32_t PmPlatformComponentManager::FileSearchWithWildCard(const std::filesystem::path &searchPath, std::vector<std::filesystem::path> &results)
{
    (void) searchPath;
    (void) results;
    return -1;
}

void PmPlatformComponentManager::NotifySystemRestart()
{
}

int32_t PmPlatformComponentManager::ApplyBultinUsersReadPermissions(const std::filesystem::path &filePath)
{
    (void) filePath;
    return -1;
}

int32_t PmPlatformComponentManager::RestrictPathPermissionsToAdmins(const std::filesystem::path &filePath)
{
    (void) filePath;
    return -1;
}
