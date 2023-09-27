#include "IPmPkgUtil.hpp"
#include "MockPmPlatformComponentManager/MockCodesignVerifier.hpp"
#include "PmPlatformComponentManager.hpp"
#include "PmLogger.hpp"
#include "UnitTestBase.h"

using namespace testing;

// Test fixture for PmPlatformComponentManager tests
class PmPlatformComponentManagerTest : public TestEnv::UnitTestBase {
public:
    PmPlatformComponentManagerTest() {
        // Initialize PmPlatformComponentManager with the mock object
        mockCodesignVerifier_ = std::make_shared<StrictMock<MockCodesignVerifier>>();
        manager_ = std::make_shared<PmPlatformComponentManager>( mockEnv_.pkgUtil_, mockCodesignVerifier_, mockEnv_.fileUtils_);
    }
    
    ~PmPlatformComponentManagerTest(){
        // Reset the mock object
        mockCodesignVerifier_.reset();
        manager_.reset();
    }
    
protected:
    // Mocked IPmPkgUtil object
    std::shared_ptr<MockCodesignVerifier> mockCodesignVerifier_;
    
    // Instance of PmPlatformComponentManager to be tested
    std::shared_ptr<PmPlatformComponentManager> manager_;
};

// Test case for GetInstalledPackages
TEST_F(PmPlatformComponentManagerTest, GetInstalledPackages) {
    // Prepare test data
    std::vector<PmProductDiscoveryRules> catalogRules = {
        { "ProductA", {}, {}, {}, {}, { {"com.test.Package1"}, {"com.test.Package2"}, {"com.test.Package2"} } }
    };
    PackageInventory packagesDiscovered;
    
    // Define expected results
    std::vector<std::string> expectedPackageList = {"com.test.Package1", "com.test.Package2"};
    PmPackageInfo expectedPackageInfo[] = {
        { "com.test.Package1", "1.0", "/path/to/package"},
        { "com.test.Package2", "2.0", "/path/to/package"},
    };
    // Set up expectations on the mock object
    EXPECT_CALL(*mockEnv_.pkgUtil_, listPackages(_))
        .WillOnce(Return(expectedPackageList));
    EXPECT_CALL(*mockEnv_.pkgUtil_, getPackageInfo("com.test.Package1", _))
        .WillOnce(Return(expectedPackageInfo[0]));
    EXPECT_CALL(*mockEnv_.pkgUtil_, getPackageInfo("com.test.Package2", _))
        .WillRepeatedly(Return(expectedPackageInfo[1]));

    // Invoke the function under test
    int32_t result = manager_->GetInstalledPackages(catalogRules, packagesDiscovered);
    
    // Verify the result and expectations
    EXPECT_EQ(result, 0);
    EXPECT_EQ(packagesDiscovered.packages.size(), 2);
    EXPECT_EQ(packagesDiscovered.packages[0].product, expectedPackageInfo[0].packageIdentifier);
    EXPECT_EQ(packagesDiscovered.packages[0].version, expectedPackageInfo[0].version);
    EXPECT_EQ(packagesDiscovered.packages[1].product, expectedPackageInfo[1].packageIdentifier);
    EXPECT_EQ(packagesDiscovered.packages[1].version, expectedPackageInfo[1].version);
//    EXPECT_EQ(packagesDiscovered.packages[0].installationPath, expectedPackageInfo.installationPath);
}

// Test case for CachedInventory
TEST_F(PmPlatformComponentManagerTest, FilledCachedInventory) {
    // Prepare test data
    std::vector<PmProductDiscoveryRules> catalogRules = {
        { "ProductA", {}, {}, {}, {}, { {"com.test.Package1"}, {"com.test.Package2"} } }
    };
    PackageInventory packagesDiscovered;
    
        // Define expected results
    std::vector<std::string> expectedPackageList = {"com.test.Package1", "com.test.Package2"};
    PmPackageInfo expectedPackageInfo[] = {
        { "com.test.Package1", "1.0", "/path/to/package"},
        { "com.test.Package2", "2.0", "/path/to/package"},
    };
        // Set up expectations on the mock object
    EXPECT_CALL(*mockEnv_.pkgUtil_, listPackages(_))
        .WillOnce(Return(expectedPackageList));
    EXPECT_CALL(*mockEnv_.pkgUtil_, getPackageInfo("com.test.Package1", _))
        .WillOnce(Return(expectedPackageInfo[0]));
    EXPECT_CALL(*mockEnv_.pkgUtil_, getPackageInfo("com.test.Package2", _))
        .WillOnce(Return(expectedPackageInfo[1]));
    
        // Invoke the function under test
    int32_t result = manager_->GetInstalledPackages(catalogRules, packagesDiscovered);
    
    PackageInventory cachedInventory;
    int32_t resultCached = manager_->GetCachedInventory(cachedInventory);
    
    // Verify the result and expectations
    EXPECT_EQ(resultCached, 0);
    EXPECT_EQ(cachedInventory.packages.size(), 2);
    EXPECT_EQ(cachedInventory.packages[0].product, expectedPackageInfo[0].packageIdentifier);
    EXPECT_EQ(cachedInventory.packages[0].version, expectedPackageInfo[0].version);
    EXPECT_EQ(cachedInventory.packages[1].product, expectedPackageInfo[1].packageIdentifier);
    EXPECT_EQ(cachedInventory.packages[1].version, expectedPackageInfo[1].version);
}

// Test case for CachedInventory
TEST_F(PmPlatformComponentManagerTest, EmptyCachedInventory) {
    // Prepare test data
    PackageInventory cachedInventory;
    int32_t resultCached = manager_->GetCachedInventory(cachedInventory);
    
    // Verify the result and expectations
    EXPECT_EQ(resultCached, 0);
    EXPECT_EQ(cachedInventory.packages.size(), 0);
}

// Test case for InstallComponent
TEST_F(PmPlatformComponentManagerTest, InstallComponent_Positive) {
    // Prepare test data
    const std::string volumePath = "/Volumes/MountedVolume";
    PmComponent package;
    package.downloadedInstallerPath = "/path/to/package.pkg";
    package.signerName = "TestSigner";
    package.installerType = "pkg";
    
    // Set up expectations on the mock object
    EXPECT_CALL(*mockEnv_.pkgUtil_,
                installPackage(
                   package.downloadedInstallerPath.u8string(),
                   _, _
                ))
        .WillOnce(Return(true));
    
    // Set up expectations on the mock object
    EXPECT_CALL(*mockEnv_.fileUtils_,
                PathIsValid(
                   package.downloadedInstallerPath
                ))
        .WillOnce(Return(true));

    // Set up expectations on the codesignVerifier
    EXPECT_CALL(*mockCodesignVerifier_,
                PackageVerify(
                    package.downloadedInstallerPath,
                    package.signerName)
                )
        .WillOnce(Return(CodeSignStatus::CODE_SIGN_OK));
    
        // Invoke the function under test
    int32_t result = manager_->InstallComponent(package);
    
        // Verify the result and expectations
    EXPECT_EQ(result, 0);
}

// Test case for InstallComponent via Update
TEST_F(PmPlatformComponentManagerTest, UpdateComponent_Positive) {
    // Prepare test data
    const std::string volumePath = "/Volumes/MountedVolume";
    PmComponent package;
    package.downloadedInstallerPath = "/path/to/package.pkg";
    package.signerName = "TestSigner";
    package.installerType = "pkg";
    
    // Set up expectations on the mock object
    EXPECT_CALL(*mockEnv_.pkgUtil_,
                installPackage(
                   package.downloadedInstallerPath.u8string(),
                   _, _
                ))
        .WillOnce(Return(true));
    
    // Set up expectations on the mock object
    EXPECT_CALL(*mockEnv_.fileUtils_,
                PathIsValid(
                   package.downloadedInstallerPath
                ))
        .WillOnce(Return(true));

    // Set up expectations on the codesignVerifier
    EXPECT_CALL(*mockCodesignVerifier_,
                PackageVerify(
                    package.downloadedInstallerPath,
                    package.signerName)
                )
        .WillOnce(Return(CodeSignStatus::CODE_SIGN_OK));
    
        // Invoke the function under test
    std::string errOut;
    ASSERT_EQ(manager_->UpdateComponent(package, errOut).pmResult, IPmPlatformComponentManager::PM_INSTALL_SUCCESS);
}

TEST_F(PmPlatformComponentManagerTest, InstallComponent_UnknownPkgType_Negative) {
    // Prepare test data
    const std::string volumePath = "/Volumes/MountedVolume";
    PmComponent package;
    package.downloadedInstallerPath = "/path/to/package.dmg";
    package.signerName = "TestSigner";
    package.installerType = "dmg";
    
    // Set up expectations on the mock object
    EXPECT_CALL(*mockEnv_.fileUtils_,
                PathIsValid(
                            package.downloadedInstallerPath
                            ))
    .WillOnce(Return(true));
    
        // Invoke the function under test
    int32_t result = manager_->InstallComponent(package);
    
    // Verify the result and expectations
    EXPECT_EQ(result, -1);
}

TEST_F(PmPlatformComponentManagerTest, InstallComponent_PastKillDate_Negative) {
    // Prepare test data
    const std::string volumePath = "/Volumes/MountedVolume";
    PmComponent package;
    package.downloadedInstallerPath = "/path/to/package.pkg";
    package.signerName = "TestSigner";
    package.installerType = "pkg";
    
    // Set up expectations on the codesignVerifier
    EXPECT_CALL(*mockCodesignVerifier_,
                PackageVerify(
                   package.downloadedInstallerPath,
                   package.signerName
                )
    )
    .WillOnce(Return(CodeSignStatus::CODE_SIGN_VERIFICATION_FAILED));
    
    // Set up expectations on the mock object
    EXPECT_CALL(*mockEnv_.fileUtils_,
                PathIsValid(
                    package.downloadedInstallerPath
                ))
    .WillOnce(Return(true));

    
    // Invoke the function under test
    int32_t result = manager_->InstallComponent(package);
    
    // TODO, oskryp: have to add enum to describe ComponentManager error codes
    // Verify the result and expectations
    EXPECT_EQ(result, -16);
}

TEST_F(PmPlatformComponentManagerTest, InstallComponent_InstallFailed_Negative) {
    // Prepare test data
    const std::string volumePath = "/Volumes/MountedVolume";
    PmComponent package;
    package.downloadedInstallerPath = "/path/to/package.pkg";
    package.signerName = "TestSigner";
    package.installerType = "pkg";
    
    // Set up expectations on the mock object
    EXPECT_CALL(*mockEnv_.pkgUtil_,
                installPackage(
                   package.downloadedInstallerPath.u8string(),
                   _, _
               ))
    .WillOnce(Return(false));
    
    // Set up expectations on the mock object
    EXPECT_CALL(*mockEnv_.fileUtils_,
                PathIsValid(
                    package.downloadedInstallerPath
                ))
    .WillOnce(Return(true));

    
    // Set up expectations on the codesignVerifier
    EXPECT_CALL(*mockCodesignVerifier_,
                PackageVerify(
                   package.downloadedInstallerPath,
                   package.signerName
                )
    )
    .WillOnce(Return(CodeSignStatus::CODE_SIGN_OK));
    
    // Invoke the function under test
    int32_t result = manager_->InstallComponent(package);
    
    // Verify the result and expectations
    EXPECT_EQ(result, -1);
}
