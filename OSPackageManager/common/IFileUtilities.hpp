/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#pragma once

#include <filesystem>

namespace PackageManager
{

class IFileUtilities{
public:
    IFileUtilities() = default;
    virtual ~IFileUtilities() = default;

    virtual bool PathIsValid(const std::filesystem::path &filePath) = 0;
    virtual bool HasAdminRestrictionsApplied(const std::filesystem::path &filePath) = 0;
    virtual bool HasUserRestrictionsApplied(const std::filesystem::path &filePath) = 0;
    virtual bool ApplyAdminRestrictions(const std::filesystem::path &filePath) = 0;
    virtual bool ApplyUserRestrictions(const std::filesystem::path &filePath) = 0;
    virtual int32_t FileSearchWithWildCard(const std::filesystem::path& searchPath, std::vector<std::filesystem::path>& results) = 0;
    virtual std::string ResolvePath(const std::string& basePath) = 0;
    virtual std::string ResolveKnownFolderIdForDefaultUser(const std::string& knownFolderId) = 0;
};

}
