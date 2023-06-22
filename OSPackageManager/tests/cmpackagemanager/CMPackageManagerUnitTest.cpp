#include <stddef.h>
#include <thread>
#include <memory>
#include <string>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../../macOS/PmPlatformConfiguration.hpp"
#include "CMLogger.hpp"
#include "MockCMIDAPIProxy.hpp"
#include "MockPmCertManager/MockPmCertRetriever.h"
#include "MockPmCertManager/MockPmCertManager.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::SetArgPointee;
using ::testing::SaveArg;
using ::testing::Return;
using ::testing::NiceMock;
using ::testing::AnyNumber;

class PmPlatformConfigurationTestWithUninitialisedCMIDAPI : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up the test fixture
        mockCMIDApi = std::make_shared<NiceMock<MockCMIDAPIProxy>>(false);
        //TODO: vzakharc - get rid of cert mgr mock in pkg mgr tests
        auto retrieverMock = std::make_shared<NiceMock<MockPmCertRetriever>>();
        auto certMgrMock = std::make_shared<NiceMock<MockPmCertManager>>(retrieverMock);
        EXPECT_CALL(*retrieverMock, LoadSystemSslCertificates()).Times(AnyNumber());
        EXPECT_CALL(*retrieverMock, FreeSystemSslCertificates()).Times(AnyNumber());
        platformConfiguration = std::make_unique<PmPlatformConfiguration>(mockCMIDApi, certMgrMock);
    }
    
    void TearDown() override {
        // Verify and clear expectations after each test
        testing::Mock::VerifyAndClearExpectations(mockCMIDApi.get());
    }
    
    // Define member variables used in the test
    std::shared_ptr<MockCMIDAPIProxy> mockCMIDApi;
    std::unique_ptr<PmPlatformConfiguration> platformConfiguration;
};

TEST_F(PmPlatformConfigurationTestWithUninitialisedCMIDAPI, CallRefreshToken) {
    EXPECT_CALL(*mockCMIDApi, refresh_token)
        .WillOnce(Return(CMID_RES_NOT_INITED));
    EXPECT_FALSE(platformConfiguration->RefreshIdentity());
}

TEST_F(PmPlatformConfigurationTestWithUninitialisedCMIDAPI, GetToken) {
    EXPECT_CALL(*mockCMIDApi, get_token)
        .WillOnce(Return(CMID_RES_NOT_INITED));
    std::string cmidToken;
    EXPECT_FALSE(platformConfiguration->GetIdentityToken(cmidToken));
}

TEST_F(PmPlatformConfigurationTestWithUninitialisedCMIDAPI, GetIdentity) {
    EXPECT_CALL(*mockCMIDApi, get_id)
        .WillOnce(Return(CMID_RES_NOT_INITED));
    std::string cmidIdentity;
    EXPECT_FALSE(platformConfiguration->GetUcIdentity(cmidIdentity));
}

TEST_F(PmPlatformConfigurationTestWithUninitialisedCMIDAPI, GetUrls) {
    EXPECT_CALL(*mockCMIDApi, get_url)
        .WillOnce(Return(CMID_RES_NOT_INITED))
        .WillOnce(Return(CMID_RES_NOT_INITED))
        .WillOnce(Return(CMID_RES_NOT_INITED));
    PmUrlList cmidUrls;
    EXPECT_FALSE(platformConfiguration->GetPmUrls(cmidUrls));
}

class PmPlatformConfigurationTestWithInitialisedCMIDAPI : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up the test fixture
        mockCMIDApi = std::make_shared<NiceMock<MockCMIDAPIProxy>>(true);
        mockCMIDApi->DelegateToFake();
        //TODO: vzakharc - get rid of cert mgr mock in pkg mgr tests
        auto retrieverMock = std::make_shared<NiceMock<MockPmCertRetriever>>();
        auto certMgrMock = std::make_shared<NiceMock<MockPmCertManager>>(retrieverMock);
        EXPECT_CALL(*retrieverMock, LoadSystemSslCertificates()).Times(AnyNumber());
        EXPECT_CALL(*retrieverMock, FreeSystemSslCertificates()).Times(AnyNumber());

        platformConfiguration = std::make_unique<PmPlatformConfiguration>(mockCMIDApi, certMgrMock);
    }
    
    void TearDown() override {
        // Verify and clear expectations after each test
        testing::Mock::VerifyAndClearExpectations(mockCMIDApi.get());
    }
    
    // Define member variables used in the test
    std::shared_ptr<MockCMIDAPIProxy> mockCMIDApi;
    std::unique_ptr<PmPlatformConfiguration> platformConfiguration;
};

TEST_F(PmPlatformConfigurationTestWithInitialisedCMIDAPI, CallRefreshToken) {
    EXPECT_TRUE(platformConfiguration->RefreshIdentity());
}

TEST_F(PmPlatformConfigurationTestWithInitialisedCMIDAPI, GetToken) {
    std::string cmidToken;
    EXPECT_TRUE(platformConfiguration->GetIdentityToken(cmidToken));
    EXPECT_EQ(cmidToken, "cmid_token");
}

TEST_F(PmPlatformConfigurationTestWithInitialisedCMIDAPI, GetIdentity) {
    std::string cmidIdentity;
    EXPECT_TRUE(platformConfiguration->GetUcIdentity(cmidIdentity));
    EXPECT_EQ(cmidIdentity, "cmid_identity");
}

TEST_F(PmPlatformConfigurationTestWithInitialisedCMIDAPI, GetUrls) {
    PmUrlList cmidUrls;
    EXPECT_TRUE(platformConfiguration->GetPmUrls(cmidUrls));
    EXPECT_EQ(cmidUrls.eventUrl, "cmid_event_url");
    EXPECT_EQ(cmidUrls.checkinUrl, "cmid_checkin_url");
    EXPECT_EQ(cmidUrls.catalogUrl, "cmid_catalog_url");
}
