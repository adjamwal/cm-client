/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#pragma once

#include "PackageManager/IPmPlatformConfiguration.h"
#include "PackageManager/IPmPlatformComponentManager.h"
#include "PackageManager/IPmPlatformDependencies.h"
#include "gmock/gmock.h"

class MockPmPlatformDependencies : public IPmPlatformDependencies
{
  public:
    MOCK_METHOD(IPmPlatformConfiguration&, Configuration, (), (override));
    MOCK_METHOD(IPmPlatformComponentManager&, ComponentManager, (), (override));
};
