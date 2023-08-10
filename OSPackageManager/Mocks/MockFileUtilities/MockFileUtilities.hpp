/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#pragma once
#include "IFileUtilities.hpp"
#include "gmock/gmock.h"

class MockFileUtilities : public PackageManager::IFileUtilities
{
  public:
    MOCK_METHOD(bool, PathIsValid, (const std::filesystem::path &filePath), (override));
    MOCK_METHOD(bool, HasAdminRestrictionsApplied, (const std::filesystem::path &filePath), (override));
    MOCK_METHOD(bool, HasUserRestrictionsApplied, (const std::filesystem::path &filePath), (override));
    MOCK_METHOD(bool, ApplyAdminRestrictions, (const std::filesystem::path &filePath), (override));
    MOCK_METHOD(bool, ApplyUserRestrictions, (const std::filesystem::path &filePath), (override));
    MOCK_METHOD(int32_t, FileSearchWithWildCard, (const std::filesystem::path&, std::vector<std::filesystem::path>&), (override));
};

