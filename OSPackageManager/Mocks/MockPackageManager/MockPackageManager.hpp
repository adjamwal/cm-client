/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#pragma once
#include "gmock/gmock.h"

#include "PackageManager/IPackageManager.h"
#include "PackageManager/IPmPlatformDependencies.h"

class MockPackageManager : public IPackageManager
{
  public:
    MOCK_METHOD(int32_t, Start, (const char* pmConfigFile, const char* pmBootstrapFile), (override));
    MOCK_METHOD(int32_t, Stop, (), (override));
    MOCK_METHOD(bool, IsRunning, (), (override));
    MOCK_METHOD(void, SetPlatformDependencies, (IPmPlatformDependencies* dependecies), (override));
    MOCK_METHOD(int32_t, VerifyPmConfig, (const char* pmConfigFile), (override));
};
