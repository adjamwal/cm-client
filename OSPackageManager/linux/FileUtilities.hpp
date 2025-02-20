/**
 * @file
 *
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved
 */

#pragma once

#include "IFileUtilities.hpp"

namespace PackageManager
{
struct LinuxSearchPathUtil {
    typedef enum {
        USER_HOME = 0
    } KNOWN_FOLDER_ID ;

    const static std::unordered_map<std::string, KNOWN_FOLDER_ID> knownFolderIdMap;
};
    
class FileUtilities : public IFileUtilities{
public:
    FileUtilities() = default;
    ~FileUtilities() = default;
    bool PathIsValid(const std::filesystem::path &filePath) override;
    bool HasAdminRestrictionsApplied(const std::filesystem::path &filePath) override;
    bool HasUserRestrictionsApplied(const std::filesystem::path &filePath) override;
    bool ApplyAdminRestrictions(const std::filesystem::path &filePath) override;
    bool ApplyUserRestrictions(const std::filesystem::path &filePath) override;
    int32_t FileSearchWithWildCard(const std::filesystem::path& searchPath, std::vector<std::filesystem::path>& results) override;
    std::string ResolvePath(const std::string& basePath) override;
    std::string ResolveKnownFolderIdForDefaultUser(const std::string& knownFolderId) override;
    
};

}
