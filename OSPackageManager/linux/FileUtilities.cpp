/**
 * @file
 *
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved
 */

#include "FileUtilities.hpp"
#include "PmLogger.hpp"

namespace PackageManager
{
bool FileUtilities::PathIsValid(const std::filesystem::path &filePath) {
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

bool FileUtilities::HasAdminRestrictionsApplied(const std::filesystem::path &filePath) {
    (void) filePath;
    return false;
}

bool FileUtilities::HasUserRestrictionsApplied(const std::filesystem::path &filePath) {
    (void) filePath;
    return false;
}

bool FileUtilities::ApplyAdminRestrictions(const std::filesystem::path &filePath) {
    (void) filePath;
    return false;
}

bool FileUtilities::ApplyUserRestrictions(const std::filesystem::path &filePath) {
    (void) filePath;
    return false;
}

int32_t FileUtilities::FileSearchWithWildCard(const std::filesystem::path& searchPath, std::vector<std::filesystem::path>& results) {

    (void) searchPath;
    (void) results;  
    int32_t dwError = 0;
    return dwError;
}

std::string FileUtilities::ResolvePath(const std::string &basePath) {   
    (void) basePath;
    return std::string{};
}

std::string FileUtilities::ResolveKnownFolderIdForDefaultUser(const std::string& knownFolderId) {
    (void) knownFolderId;
    return std::string{};
}

}
