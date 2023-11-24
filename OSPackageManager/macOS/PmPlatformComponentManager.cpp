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

#include <sstream>
#include <map>
#include <cassert>

namespace { // anonymous namespace

std::map<std::string, int> sanitizeParamsXml (const std::string& args)
{
    std::map<std::string, int> resultArg;
    const std::string enterName("installer_choices=\"");
    auto firstOcc = args.find(enterName);
    if ( std::string::npos == firstOcc )
        return resultArg;
    
    auto secondOcc = args.find("\"", firstOcc+enterName.length());
    if ( std::string::npos == secondOcc )
        return resultArg;
    
    if (firstOcc+1 >= secondOcc)
        return resultArg;

    std::string theArgs(args, firstOcc+enterName.length(), secondOcc - (firstOcc+enterName.length()));
    const char delim = ' ';
    std::istringstream ss(theArgs);
    std::string token;
    std::vector<std::string> tokens;
    while(std::getline(ss, token, delim)) {
        std::istringstream ssInner(token);
        std::string key, val;
        std::getline(ssInner, key, '=');
        std::getline(ssInner, val, '=');

        resultArg[key] = std::stoi(val);
    }
    
    return resultArg;
};

bool containsRebootInstruction(const std::string& sArgs)
{
    static const std::vector<std::string> vRebootRestart = {"SR=true", "FR=true"};
    for (auto& arg: vRebootRestart) {
        if (std::string::npos != sArgs.find_first_of(arg))
            return true;
    }
        
    return  false;
}

}

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
    if( package.installerType == "pkg" && !package.signerName.empty() ) {
        status = codesignVerifier_->PackageVerify(
            downloadedInstallerPath,
            package.signerName
        );
    }
    
    if( status == CodeSignStatus::CODE_SIGN_OK )
    {
        if( package.installerType == "pkg" )
        {
            const auto success = pkgUtil_->installPackage(downloadedInstallerPath, sanitizeParamsXml(package.installerArgs));
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
    
    PM_LOG_INFO("Package installation status %d", ret);
    return ret;
}

IPmPlatformComponentManager::PmInstallResult PmPlatformComponentManager::UpdateComponent(const PmComponent &package, std::string &error)
{
    (void) error;
    const bool bRebootReq = containsRebootInstruction(package.installerArgs);

    IPmPlatformComponentManager::PmInstallResult result = {
        .pmResult = InstallComponent(package) == 0
                    ? (bRebootReq ? IPmPlatformComponentManager::PM_INSTALL_SUCCESS_REBOOT_REQUIRED: IPmPlatformComponentManager::PM_INSTALL_SUCCESS)
                    : (bRebootReq ? IPmPlatformComponentManager::PM_INSTALL_FAILURE_REBOOT_REQUIRED :IPmPlatformComponentManager::PM_INSTALL_FAILURE),
        .platformResult = 0
    };

    return result;
}

int32_t PmPlatformComponentManager::UninstallComponent(const PmComponent &package)
{
    if (!fileUtils_->PathIsValid(package.uninstallerLocation))
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
    CodeSignStatus status = CodeSignStatus::CODE_SIGN_VERIFICATION_FAILED;
    
    std::filesystem::path UnInstallerPath = package.uninstallerLocation;
    UnInstallerPath.make_preferred();
    if( !package.uninstallerSignerName.empty() ) {
        status = codesignVerifier_->PackageVerify(
            UnInstallerPath,
            package.uninstallerSignerName
        );
    }

    if( status == CodeSignStatus::CODE_SIGN_OK ) {
            const auto success = pkgUtil_->installPackage(package.uninstallerLocation);
            ret = success ? 0 : -1;
    }
    else {
        const auto bShellResult = pkgUtil_->invokeShell(package.uninstallerLocation, package.uninstallerArgs);
        ret = bShellResult ? 0 : -1;
        if ( -1 == ret ) {
            PM_LOG_ERROR("Invoking %s with args '%s' failed: ",
                         package.uninstallerLocation.string().c_str(), package.uninstallerArgs.c_str() );
        }
    }
    
    PM_LOG_INFO("Package uninstallation status %d", ret);
    return ret;
}

int32_t PmPlatformComponentManager::DeployConfiguration(const PackageConfigInfo &config)
{
    (void) config;
    return 0;
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
