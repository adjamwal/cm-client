/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#pragma once
#include "IPmPlatformDiscovery.hpp"
#include "gmock/gmock.h"

class MockIPmPlatformDiscovery : public IPmPlatformDiscovery
{
  public:
    MOCK_METHOD(PackageInventory, DiscoverInstalledPackages, (const std::vector<PmProductDiscoveryRules> &catalogRules), (override));
};
