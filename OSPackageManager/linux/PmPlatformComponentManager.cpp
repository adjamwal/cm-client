/**
 * @file
 *
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved
 */

#include "PmPlatformComponentManager.hpp"
#include "PmLogger.hpp"
#include "FileUtilities.hpp"
#include "PackageManager/PmTypes.h"
#include <cassert>

PmPlatformComponentManager::PmPlatformComponentManager(
    std::shared_ptr<IPackageUtil> pkgUtil,
    std::shared_ptr<PackageManager::IFileUtilities> fileUtils) 
    : pkgUtil_(pkgUtil),  discovery_(pkgUtil, fileUtils), fileUtils_(std::move(fileUtils)){
        assert(pkgUtil_);
        assert(fileUtils_);
        if(!pkgUtil_ || !fileUtils_) {
            throw std::runtime_error("Invalid arguments");
        }
    }

int32_t PmPlatformComponentManager::GetInstalledPackages(const std::vector<PmProductDiscoveryRules> &catalogRules, PackageInventory &packagesDiscovered) {
    
    try {
        packagesDiscovered = discovery_.DiscoverInstalledPackages(catalogRules);
    } catch (PkgUtilException& e) {
        PM_LOG_ERROR("Exception: [%s]", e.what());
        return -1;
    }

    return 0;
}

int32_t PmPlatformComponentManager::GetCachedInventory(PackageInventory &cachedInventory) {
    cachedInventory = discovery_.CachedInventory();
    return 0;
}

int32_t PmPlatformComponentManager::InstallComponent(const PmComponent &package) {
    // To-Do: Implement CodeSign Verification later.
    int32_t ret = -1;

    if (!fileUtils_->PathIsValid(package.downloadedInstallerPath))
        return ret;

    
    if( pkgUtil_->isValidInstallerType(package.installerType) ) {
        const bool success = pkgUtil_->installPackage(package.downloadedInstallerPath);
        ret = success ? 0 : -1;
    } else {
        PM_LOG_ERROR("Invalid Package Type: %s", package.installerType.c_str());
    }
    
    PM_LOG_INFO("Package installation status %d", ret);
    return ret;
}

IPmPlatformComponentManager::PmInstallResult PmPlatformComponentManager::UpdateComponent(const PmComponent &package, std::string &error) {
    (void) error;
    //TODO: Re-start required handling
    IPmPlatformComponentManager::PmInstallResult result = {
        .pmResult = (InstallComponent(package) == 0)
                    ? IPmPlatformComponentManager::PM_INSTALL_SUCCESS : IPmPlatformComponentManager::PM_INSTALL_FAILURE,
        .platformResult = 0
    };
    return result;
}

int32_t PmPlatformComponentManager::UninstallComponent(const PmComponent &package) {
    (void) package;
    return 0;
}

int32_t PmPlatformComponentManager::DeployConfiguration(const PackageConfigInfo &config) {
    (void) config;
    return 0;
}

std::string PmPlatformComponentManager::ResolvePath(const std::string &basePath) {
    (void) basePath;
    return std::string{};
}

int32_t PmPlatformComponentManager::FileSearchWithWildCard(const std::filesystem::path &searchPath, std::vector<std::filesystem::path> &results) {
    (void) searchPath;
    (void) results;
    return 0;
}

void PmPlatformComponentManager::NotifySystemRestart() {
}

int32_t PmPlatformComponentManager::ApplyBultinUsersReadPermissions(const std::filesystem::path &filePath)
{
    if (!fileUtils_->PathIsValid(filePath))
        return -1;
    
    if ( !fileUtils_->HasUserRestrictionsApplied(filePath) ) {
        return fileUtils_->ApplyUserRestrictions(filePath) ? 0 : -1;
    }

    return 0;
}

int32_t PmPlatformComponentManager::RestrictPathPermissionsToAdmins(const std::filesystem::path &filePath)
{
    int32_t nRet = -1;
    if (!fileUtils_->PathIsValid(filePath))
        return  nRet;

    if ( !fileUtils_->HasAdminRestrictionsApplied(filePath) ) {
        nRet = fileUtils_->ApplyAdminRestrictions(filePath) ? 0 : -1;
    }
    else
        nRet = 0;

    return nRet;
}
