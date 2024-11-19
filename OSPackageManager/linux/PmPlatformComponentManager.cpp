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
    return 0;
}

int32_t PmPlatformComponentManager::GetCachedInventory(PackageInventory &cachedInventory)
{
    (void) cachedInventory;
    return 0;
}

int32_t PmPlatformComponentManager::InstallComponent(const PmComponent &package)
{
    (void) package;
    return 0;
}

IPmPlatformComponentManager::PmInstallResult PmPlatformComponentManager::UpdateComponent(const PmComponent &package, std::string &error)
{
    (void) package;
    (void) error;

    IPmPlatformComponentManager::PmInstallResult result = {
        .pmResult = IPmPlatformComponentManager::PM_INSTALL_FAILURE,
        .platformResult = 0
    };

    return result;
}

int32_t PmPlatformComponentManager::UninstallComponent(const PmComponent &package)
{
    (void) package;
    return 0;
}

int32_t PmPlatformComponentManager::DeployConfiguration(const PackageConfigInfo &config)
{
    (void) config;
    return 0;
}

std::string PmPlatformComponentManager::ResolvePath(const std::string &basePath)
{
    (void) basePath;
    return std::string{};
}

int32_t PmPlatformComponentManager::FileSearchWithWildCard(const std::filesystem::path &searchPath, std::vector<std::filesystem::path> &results)
{
    (void) searchPath;
    (void) results;
    return 0;
}

void PmPlatformComponentManager::NotifySystemRestart()
{
}

int32_t PmPlatformComponentManager::ApplyBultinUsersReadPermissions(const std::filesystem::path &filePath)
{
    (void) filePath;
    return 0;
}

int32_t PmPlatformComponentManager::RestrictPathPermissionsToAdmins(const std::filesystem::path &filePath)
{
    (void) filePath;
    return -1;
}
