/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "PmPlatformComponentManager.hpp"
#include "PackageManager/PmTypes.h"
#include "PmLogger.hpp"

PmPlatformComponentManager::PmPlatformComponentManager(std::shared_ptr<IPmPkgUtil> pkgUtil) : discovery_(pkgUtil) {}

int32_t PmPlatformComponentManager::GetInstalledPackages(const std::vector<PmProductDiscoveryRules> &catalogRules, PackageInventory &packagesDiscovered)
{
    try {
        packagesDiscovered = discovery_.DiscoverInstalledPackages(catalogRules);
    } catch (PkgUtilException& e) {
        PmLogger::getLogger().Log(IPMLogger::LOG_ERROR, "Exception: [%s]", e.what());
        return -1;
    }
    return 0;
}

int32_t PmPlatformComponentManager::GetCachedInventory(PackageInventory &cachedInventory)
{
    cachedInventory = discovery_.CachedInventory();
    return 0;
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
