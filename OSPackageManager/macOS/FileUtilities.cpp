/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#include <iostream>
#include <glob.h>
#include <regex>
#include "FileUtilities.hpp"
#include "PmLogger.hpp"

namespace PackageManager
{
bool FileUtilities::PathIsValid(const std::filesystem::path &filePath)
{
    bool bValid = false;
    try{
        bValid = !filePath.empty() && std::filesystem::exists(filePath);
    }
    catch (std::filesystem::filesystem_error &err){
        bValid = false;
        PM_LOG_ERROR("PathIsValid failed on file:\"%s\" with code: %d and message: \"%s\"", filePath.c_str(), err.code().value(), err.what());
    }
    
    return bValid;
}

bool FileUtilities::HasAdminRestrictionsApplied(const std::filesystem::path &filePath)
{
    if (!PathIsValid(filePath))
        return false;
    
    const auto pathPermissions = std::filesystem::status(filePath).permissions();
 
    //x flag is not mandatory
    bool bAdminHasAccess = std::filesystem::perms::owner_read == (pathPermissions & std::filesystem::perms::owner_read);
    bAdminHasAccess &= std::filesystem::perms::owner_write == (pathPermissions & std::filesystem::perms::owner_write);
    
    //r flag is allowed
    bool bOthersHaveAccess = std::filesystem::perms::others_write == (pathPermissions & std::filesystem::perms::others_write);
    bOthersHaveAccess |= std::filesystem::perms::others_exec == (pathPermissions & std::filesystem::perms::others_exec);
    
    bool bGroupHaveAccess = std::filesystem::perms::group_write == (pathPermissions & std::filesystem::perms::group_write);
    bGroupHaveAccess |= std::filesystem::perms::group_exec == (pathPermissions & std::filesystem::perms::group_exec);
    
    return bAdminHasAccess && !bOthersHaveAccess && !bGroupHaveAccess;
}

bool FileUtilities::HasUserRestrictionsApplied(const std::filesystem::path &filePath)
{
    if (!PathIsValid(filePath))
        return false;
    
    const auto pathPermissions = std::filesystem::status(filePath).permissions();
    
    // Check if others have read permission
    const bool bOthersHaveAccess = std::filesystem::perms::others_read == (pathPermissions & std::filesystem::perms::others_read);
    
    return bOthersHaveAccess;
}

bool FileUtilities::ApplyAdminRestrictions(const std::filesystem::path &filePath)
{
    if (!PathIsValid(filePath))
        return false;
    
    if (HasAdminRestrictionsApplied(filePath))
        return true;
    
    //Remove extra permissions
    std::error_code errCode;
    try{
        std::filesystem::permissions(filePath, std::filesystem::perms::others_write
                                     | std::filesystem::perms::others_exec
                                     | std::filesystem::perms::group_write
                                     | std::filesystem::perms::group_exec,
                                     std::filesystem::perm_options::remove, errCode);
    }
    catch(std::filesystem::filesystem_error &err){
        PM_LOG_ERROR("ApplyAdminRestrictions failed removing permissions on file:\"%s\" with code: %d and message: \"%s\"", filePath.c_str(), err.code().value(), err.what());
        return false;
    }
    
    try{
        std::filesystem::permissions(filePath, std::filesystem::perms::owner_read | std::filesystem::perms::owner_write,
                                     std::filesystem::perm_options::add, errCode);
    }
    catch(std::filesystem::filesystem_error &err){
        PM_LOG_ERROR("ApplyAdminRestrictions failed adding permissions on file:\"%s\" with code: %d and message: \"%s\"", filePath.c_str(), err.code().value(), err.what());
        return false;
    }
    
    return true;
}

bool FileUtilities::ApplyUserRestrictions(const std::filesystem::path &filePath)
{
    if (!PathIsValid(filePath))
        return false;
    
    // Add read permission for others
    std::error_code errCode;
    std::filesystem::permissions(filePath, std::filesystem::perms::others_read,
       std::filesystem::perm_options::add, errCode);
    
    if (errCode) {
        PM_LOG_ERROR("ApplyUserRestrictions failed adding permissions on file:\"%s\" with code: %d and message: \"%s\"", filePath.c_str(), errCode.value(), errCode.message().c_str());
        return false;
    }
    
    return true;
}

int32_t FileUtilities::FileSearchWithWildCard(const std::filesystem::path& searchPath, std::vector<std::filesystem::path>& results) {

    const std::string searchPattern = searchPath.string();
    glob_t globResult;
    int32_t dwError = glob(searchPattern.c_str(), 0, nullptr, &globResult);
    
    if (dwError == 0) {
        for (size_t i = 0; i < globResult.gl_pathc; ++i) {
            std::filesystem::path filePath(globResult.gl_pathv[i]);
            results.push_back(filePath);
        }
        globfree(&globResult);
    } else {
        PM_LOG_ERROR("glob with searchPattern=%s returns %d", searchPattern.c_str(), dwError);
    }
    
    return dwError;
}

std::string FileUtilities::ResolvePath(const std::string &basePath)
{
    const std::regex folderIdRegex(R"(<FOLDERID_([^>]+)>)");
    std::smatch matches;
    
    if (std::regex_search(basePath, matches, folderIdRegex)) {
        const auto& folderIdString = matches[1].str();
        const auto knownFolder = ResolveKnownFolderIdForDefaultUser(folderIdString);
        
        if (!knownFolder.empty()) {
            return std::regex_replace(basePath, folderIdRegex, knownFolder);
        } else {
            PM_LOG_WARNING("Failed to resolve path %s", basePath.c_str());
        }
    }
    
    return basePath;
}

}
