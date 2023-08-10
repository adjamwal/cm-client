/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#pragma once
#include "PackageManager/IPmPlatformComponentManager.h"
#include "gmock/gmock.h"

class MockPmPlatformComponentManager : public IPmPlatformComponentManager
{
  public:
    MOCK_METHOD(int32_t, GetInstalledPackages, (const std::vector<PmProductDiscoveryRules>& catalogRules, PackageInventory& packagesDiscovered), (override));
    MOCK_METHOD(int32_t, GetCachedInventory, (PackageInventory& cachedInventory), (override));
    MOCK_METHOD(int32_t, InstallComponent, (const PmComponent& package), (override));
    MOCK_METHOD(PmInstallResult, UpdateComponent, (const PmComponent& package, std::string& error), (override));
    MOCK_METHOD(int32_t, UninstallComponent, (const PmComponent& package), (override));
    MOCK_METHOD(int32_t, DeployConfiguration, (const PackageConfigInfo& config), (override));
    MOCK_METHOD(std::string, ResolvePath, (const std::string& basePath), (override));
    MOCK_METHOD(int32_t, FileSearchWithWildCard, (const std::filesystem::path&, std::vector<std::filesystem::path>&), (override));
    MOCK_METHOD(void, NotifySystemRestart, (), (override));
    MOCK_METHOD(int32_t, ApplyBultinUsersReadPermissions, (const std::filesystem::path& filePath), (override));
    MOCK_METHOD(int32_t, RestrictPathPermissionsToAdmins, (const std::filesystem::path& filePath), (override));
};
