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
        PM_LOG_ERROR("Exception thrown while validating file path:\"%s\" with code: %d and message: \"%s\"", filePath.c_str(), err.code().value(), err.what());
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

std::string FileUtilities::ResolvePath(const std::string &basePath) {   
    (void) basePath;
    return std::string{};
}

std::string FileUtilities::ResolveKnownFolderIdForDefaultUser(const std::string& knownFolderId) {
    (void) knownFolderId;
    return std::string{};
}

}
