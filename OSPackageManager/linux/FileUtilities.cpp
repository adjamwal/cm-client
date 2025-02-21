/**
 * @file
 *
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved
 */

#include "FileUtilities.hpp"
#include "PmLogger.hpp"
#include <pwd.h>
#include <utmp.h>
#include <signal.h>
#include <string.h>
#include <regex>

namespace { //anonymous namespace

// NOTE: This function is not thread safe.
bool GetCurrentConsoleUser(std::string &userName) {
    userName.erase();
    setutent();
    struct utmp *entry = nullptr;
    do {
        entry = getutent();
        if (NULL != entry && USER_PROCESS == entry->ut_type && 
        0 < entry->ut_pid && 
        0 == kill(entry->ut_pid,0) && 
        (0 != strcmp("(unknown)", entry->ut_name))) {
            userName = entry->ut_name;
            break;
        }
    } while (NULL != entry);
    endutent();
    return !userName.empty();
}

void ResolveUserHomeFolder(std::string &userHomeFolder) {
    std::string userName;
    if (!GetCurrentConsoleUser(userName)) {
        PM_LOG_ERROR("Failed to get current console user");
        return;
    }

    struct passwd *pw = getpwnam(userName.c_str());
    if (pw == NULL || pw->pw_dir == NULL) {
        PM_LOG_ERROR("Failed to get user home directory for user %s", userName.c_str());
        return;
    }

    userHomeFolder = std::string(pw->pw_dir);
}

}

namespace PackageManager
{
const std::unordered_map<std::string, std::function<void(std::string &)>> LinuxSearchPathUtil::knownFolderIdMap = {
    {"UserHome", ResolveUserHomeFolder}
};

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

    (void) searchPath;
    (void) results;  
    int32_t dwError = 0;
    return dwError;
}

/**
 * @brief Resolves the path by replacing the folder ID with the actual path.
 * @note  The folder ID should be in the format <FOLDERID_xxx>, where xxx is the supported known folder ID.
 * @note  If the format is not correct or the folder ID is not supported, the function will return the base path as is.
 */
std::string FileUtilities::ResolvePath(const std::string &basePath) {   
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

std::string FileUtilities::ResolveKnownFolderIdForDefaultUser(const std::string& knownFolderId) {
    auto it = LinuxSearchPathUtil::knownFolderIdMap.find(knownFolderId);
    if (it == LinuxSearchPathUtil::knownFolderIdMap.end()) {
        PM_LOG_WARNING("Known folder ID %s not found in map", knownFolderId.c_str());
        return std::string {};
    } 

    std::string folderPath {};
    it->second(folderPath);

    return folderPath;
}

}
