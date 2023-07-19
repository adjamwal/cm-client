/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#include "MockEnvironment.h"
#include "PmLogger.hpp"

using testing::ReturnRef;

namespace TestEnv
{

//TODO: Viacheslav - get rid of real logger in unit tests
MockEnvironmentBase::MockEnvironmentBase()
    :platformDependencies_(std::make_shared<::testing::StrictMock<MockPmPlatformDependencies>>()),
    platformConfiguration_(std::make_shared<::testing::StrictMock<MockPmPlatformConfiguration>>()),
    platformCompMgr_(std::make_shared<::testing::StrictMock<MockPmPlatformComponentManager>>()),
    certRetriever_(std::make_shared<::testing::StrictMock<MockPmCertRetriever>>()),
    pkgUtil_(std::make_shared<::testing::StrictMock<MockPmPkgUtil>>()),
    pkgMgr_(std::make_shared<::testing::StrictMock<MockPackageManager>>()),
    cmdiProxy_(std::make_shared<::testing::StrictMock<MockCMIDAPIProxyAbstract>>()),
    fileUtils_(std::make_shared<::testing::StrictMock<MockFileUtilities>>())
{
    EXPECT_CALL(*platformDependencies_, Configuration()).WillRepeatedly(ReturnRef(*platformConfiguration_));
    EXPECT_CALL(*platformDependencies_, ComponentManager()).WillRepeatedly(ReturnRef(*platformCompMgr_));
    
    PmLogger::initLogger();
}

MockEnvironmentBase::~MockEnvironmentBase()
{
    testing::Mock::VerifyAndClearExpectations(&platformDependencies_);
    testing::Mock::VerifyAndClearExpectations(&platformConfiguration_);
    testing::Mock::VerifyAndClearExpectations(&platformCompMgr_);
    testing::Mock::VerifyAndClearExpectations(&certRetriever_);
    testing::Mock::VerifyAndClearExpectations(&pkgUtil_);
    testing::Mock::VerifyAndClearExpectations(&pkgMgr_);
    testing::Mock::VerifyAndClearExpectations(&cmdiProxy_);
    testing::Mock::VerifyAndClearExpectations(&fileUtils_);
    
    platformDependencies_.reset();
    platformConfiguration_.reset();
    platformCompMgr_.reset();
    certRetriever_.reset();
    pkgUtil_.reset();
    pkgMgr_.reset();
    cmdiProxy_.reset();
    fileUtils_.reset();
    
    PmLogger::releaseLogger();
}

}
