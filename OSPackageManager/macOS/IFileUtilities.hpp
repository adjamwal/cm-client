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

};

}
