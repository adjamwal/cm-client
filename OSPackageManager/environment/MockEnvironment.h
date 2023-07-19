/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#pragma once

#include "MockCMIDAPIProxyAbstract/MockCMIDAPIProxyAbstract.hpp"
#include "MockFileUtilities/MockFileUtilities.hpp"
#include "MockPackageManager/MockPackageManager.hpp"
#include "MockPmCertManager/MockPmCertRetriever.h"
#include "MockPmPlatformDependencies/MockPmPlatformDependencies.hpp"
#include "MockPmPlatformComponentManager/MockPmPkgUtil.hpp"
#include "MockPmPlatformComponentManager/MockPmPlatformConfiguration.hpp"
#include "MockPmPlatformComponentManager/MockPmPlatformComponentManager.hpp"
#include "gmock/gmock.h"

namespace TestEnv
{
class MockEnvironmentBase
{
public:
    std::shared_ptr<::testing::StrictMock<MockPmPlatformDependencies>> platformDependencies_;
    std::shared_ptr<::testing::StrictMock<MockPmPlatformConfiguration>> platformConfiguration_;
    std::shared_ptr<::testing::StrictMock<MockPmPlatformComponentManager>> platformCompMgr_;
    std::shared_ptr<::testing::StrictMock<MockPmCertRetriever>> certRetriever_;
    std::shared_ptr<::testing::StrictMock<MockPmPkgUtil>> pkgUtil_;
    std::shared_ptr<::testing::StrictMock<MockPackageManager>> pkgMgr_;
    std::shared_ptr<::testing::StrictMock<MockCMIDAPIProxyAbstract>> cmdiProxy_;
    std::shared_ptr<::testing::StrictMock<MockFileUtilities>> fileUtils_;
    
protected:
    MockEnvironmentBase();
    virtual ~MockEnvironmentBase();
};

class MockEnvironment: public MockEnvironmentBase
{
public:
    MockEnvironment() = default;
    ~MockEnvironment() = default;

};
}
