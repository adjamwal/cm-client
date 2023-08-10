/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "PmPlatformComponentManager.hpp"
#include "PackageManager/PmTypes.h"
#include "PmLogger.hpp"
#include "FileUtilities.hpp"
#include "IPmCodesignVerifier.hpp"
#include "IPmPkgUtil.hpp"

PmPlatformComponentManager::PmPlatformComponentManager(
    std::shared_ptr<IPmPkgUtil> pkgUtil,
    std::shared_ptr<IPmCodesignVerifier> codesignVerifier,
    std::shared_ptr<PackageManager::IFileUtilities> fileUtils)
: pkgUtil_(pkgUtil), codesignVerifier_(codesignVerifier), discovery_(pkgUtil), fileUtils_(fileUtils)
{}

int32_t PmPlatformComponentManager::GetInstalledPackages(const std::vector<PmProductDiscoveryRules> &catalogRules, PackageInventory &packagesDiscovered)
{
    try {
        packagesDiscovered = discovery_.DiscoverInstalledPackages(catalogRules);
    } catch (PkgUtilException& e) {
        PM_LOG_ERROR("Exception: [%s]", e.what());
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
    if (!fileUtils_->PathIsValid(package.downloadedInstallerPath))
        return -1;
    
    assert(codesignVerifier_);
    if (!codesignVerifier_) {
        PM_LOG_ERROR("No valid codesign verifier");
        return -1;
    }
    
    assert(pkgUtil_);
    if (!pkgUtil_) {
        PM_LOG_ERROR("No valid pkgUtil");
        return -1;
    }

    int32_t ret = 0;
    CodeSignStatus status = CodeSignStatus::CODE_SIGN_OK;
    
    std::filesystem::path downloadedInstallerPath = package.downloadedInstallerPath;
    downloadedInstallerPath.make_preferred();
    if( !package.signerName.empty() ) {
        status = codesignVerifier_->Verify(
            downloadedInstallerPath,
            package.signerName,
            SIGTYPE_DEFAULT );
    }
    
    if( status == CodeSignStatus::CODE_SIGN_OK )
    {
        if( package.installerType == "pkg" )
        {
            const auto success = pkgUtil_->installPackage(downloadedInstallerPath);
            ret = success ? 0 : -1;
        }
        else
        {
            PM_LOG_ERROR("Invalid Package Type: %s", package.installerType.c_str());
            ret = -1;
        }
    }
    else
    {
        PM_LOG_ERROR( "Could not verify Package." );
        ret = static_cast<int32_t>( status );
    }
    
    PM_LOG_DEBUG("Package installation status %d", ret);
    return ret;
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
    assert(fileUtils_);
    return fileUtils_->ResolvePath(basePath);
}

int32_t PmPlatformComponentManager::FileSearchWithWildCard(const std::filesystem::path &searchPath, std::vector<std::filesystem::path> &results)
{
    assert(fileUtils_);
    return fileUtils_->FileSearchWithWildCard(searchPath, results);
}

void PmPlatformComponentManager::NotifySystemRestart()
{
}

int32_t PmPlatformComponentManager::ApplyBultinUsersReadPermissions(const std::filesystem::path &filePath)
{
    if ( !fileUtils_ || !fileUtils_->PathIsValid(filePath))
        return -1;
    
    if ( !fileUtils_->HasUserRestrictionsApplied(filePath) ) {
        return fileUtils_->ApplyUserRestrictions(filePath) ? 0 : -1;
    }

    return 0;
}

int32_t PmPlatformComponentManager::RestrictPathPermissionsToAdmins(const std::filesystem::path &filePath)
{
    int32_t nRet = -1;
    if ( !fileUtils_ || !fileUtils_->PathIsValid(filePath))
        return  nRet;

    //0 refers to ERROR_SUCCESS 0 (0x0) in WINAPI
    if ( !fileUtils_->HasAdminRestrictionsApplied(filePath) ){
        nRet = fileUtils_->ApplyAdminRestrictions(filePath) ? 0 : -1;
    }
    else
        nRet = 0;

    return nRet;
}
