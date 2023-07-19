/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#pragma once

#include "IFileUtilities.hpp"

namespace PackageManager
{

class FileUtilities : public IFileUtilities{
public:
    FileUtilities() = default;
    ~FileUtilities() = default;
    bool PathIsValid(const std::filesystem::path &filePath) override;
    bool HasAdminRestrictionsApplied(const std::filesystem::path &filePath) override;
    bool HasUserRestrictionsApplied(const std::filesystem::path &filePath) override;
    bool ApplyAdminRestrictions(const std::filesystem::path &filePath) override;
    bool ApplyUserRestrictions(const std::filesystem::path &filePath) override;
};

}
