#include "IPmPkgUtil.hpp"
#include "MockPmPlatformComponentManager/MockCodesignVerifier.hpp"
#include "PmPlatformComponentManager.hpp"
#include "PmLogger.hpp"
#include "UnitTestBase.h"

using namespace testing;
using testing::SizeIs;
using testing::IsTrue;


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
    
    //Data
    const std::string testPack1_{"com.test.Package1"};
    const std::string testPack2_{"com.test.Package2"};
    const std::vector<std::string> expectedPackageList_{testPack1_, testPack2_};
    
    const std::vector<PmPackageInfo> expectedPackageInfo_{
        { "ProductA", "2.0", "/path/to/package"}
    };

    const std::vector<PmProductDiscoveryRules> catalogRules_ = {
        { "ProductA", {}, {}, {}, {}, { {testPack1_}, {testPack2_}, {testPack2_} } }
    };

    const std::string scm_json_{"/opt/cisco/secureclient/cloudmanagement/etc/cm_config.json"};
    const std::filesystem::path json_path_{scm_json_};
    const std::vector<std::filesystem::path> jsonarray_ {scm_json_};
};

// Test case for GetDeplInstalledPackages
TEST_F(PmPlatformComponentManagerTest, GetDeployPaths) {
    // Prepare test data
    PmProductDiscoveryConfigurable config = {
        {}, {}, 
        scm_json_, 
        scm_json_, 1, true, {"json", "cm"}
    };
    
    std::vector<PmProductDiscoveryRules> catalogRules(catalogRules_);
    catalogRules[0].configurables.push_back(config);
    PackageInventory packagesDiscovered;
    
    // Set up expectations on the mock object
    EXPECT_CALL(*mockEnv_.pkgUtil_, listPackages(_))
        .WillOnce(Return(expectedPackageList_));
    EXPECT_CALL(*mockEnv_.pkgUtil_, getPackageInfo(testPack1_, _))
        .WillOnce(Return(expectedPackageInfo_[0]));
    EXPECT_CALL(*mockEnv_.pkgUtil_, getPackageInfo(testPack2_, _))
        .WillRepeatedly(Return(expectedPackageInfo_[0]));
    
    EXPECT_CALL(*mockEnv_.fileUtils_, FileSearchWithWildCard( _, _ ) )
        .WillRepeatedly(
             ::testing::DoAll(SetArgReferee<1>(jsonarray_), Return(0) )
         );

    EXPECT_CALL(*mockEnv_.fileUtils_, PathIsValid( _ ) )
        .WillRepeatedly( Return( true ) );

    // Invoke the function under test
    int32_t result = manager_->GetInstalledPackages(catalogRules, packagesDiscovered);
    
    ASSERT_THAT( packagesDiscovered.packages[0].configs, SizeIs( 1 ) );
    
    EXPECT_EQ( packagesDiscovered.packages[0].configs[0].deployPath, json_path_ );
    EXPECT_EQ( packagesDiscovered.packages[0].configs[0].unresolvedDeployPath, json_path_ );
    EXPECT_THAT( packagesDiscovered.packages[0].configs[0].cfgPath.empty(), IsTrue() );
    EXPECT_THAT( packagesDiscovered.packages[0].configs[0].unresolvedCfgPath.empty(), IsTrue() );
}

// Test case for GetInstalledPackages
TEST_F(PmPlatformComponentManagerTest, GetConfigPaths) {
    // Prepare test data
    PmProductDiscoveryConfigurable config = {
        scm_json_, 
        scm_json_, {}, {}, 1, true, {"json", "cm"}
    };
    
    std::vector<PmProductDiscoveryRules> catalogRules(catalogRules_);
    catalogRules[0].configurables.push_back(config);
    PackageInventory packagesDiscovered;
    
    // Set up expectations on the mock object
    EXPECT_CALL(*mockEnv_.pkgUtil_, listPackages(_))
        .WillOnce(Return(expectedPackageList_));
    EXPECT_CALL(*mockEnv_.pkgUtil_, getPackageInfo(testPack1_, _))
        .WillOnce(Return(expectedPackageInfo_[0]));
    EXPECT_CALL(*mockEnv_.pkgUtil_, getPackageInfo(testPack2_, _))
        .WillRepeatedly(Return(expectedPackageInfo_[0]));
    
    EXPECT_CALL(*mockEnv_.fileUtils_, FileSearchWithWildCard( _, _ ) )
        .WillRepeatedly(
             ::testing::DoAll(SetArgReferee<1>(jsonarray_), Return(0) )
         );

    EXPECT_CALL(*mockEnv_.fileUtils_, PathIsValid( _ ) )
        .WillRepeatedly( Return( true ) );

    // Invoke the function under test
    int32_t result = manager_->GetInstalledPackages( catalogRules, packagesDiscovered );
    
    ASSERT_THAT( packagesDiscovered.packages[0].configs, SizeIs( 1 ) );
    
    EXPECT_EQ( packagesDiscovered.packages[0].configs[0].cfgPath, json_path_ );
    EXPECT_EQ( packagesDiscovered.packages[0].configs[0].unresolvedCfgPath, json_path_ );
    EXPECT_THAT( packagesDiscovered.packages[0].configs[0].deployPath.empty(), IsTrue() );
    EXPECT_THAT( packagesDiscovered.packages[0].configs[0].unresolvedDeployPath.empty(), IsTrue() );
}


// Test case for GetInstalledPackages
TEST_F(PmPlatformComponentManagerTest, GetInstalledPackages) {
    // Prepare test data
    std::vector<PmProductDiscoveryRules> catalogRules(catalogRules_);
    PackageInventory packagesDiscovered;
    
    // Set up expectations on the mock object
    EXPECT_CALL(*mockEnv_.pkgUtil_, listPackages(_))
        .WillOnce(Return(expectedPackageList_));
    EXPECT_CALL(*mockEnv_.pkgUtil_, getPackageInfo(testPack1_, _))
        .WillOnce(Return(expectedPackageInfo_[0]));
    EXPECT_CALL(*mockEnv_.pkgUtil_, getPackageInfo(testPack2_, _))
        .WillRepeatedly(Return(expectedPackageInfo_[0]));
    
    // Invoke the function under test
    int32_t result = manager_->GetInstalledPackages( catalogRules, packagesDiscovered );
    
    // Verify the result and expectations
    EXPECT_EQ(result, 0);
    EXPECT_EQ(packagesDiscovered.packages.size(), 1);
    EXPECT_EQ(packagesDiscovered.packages[0].product, expectedPackageInfo_[0].packageIdentifier);
    EXPECT_EQ(packagesDiscovered.packages[0].version, expectedPackageInfo_[0].version);
}

// Test case for CachedInventory
TEST_F(PmPlatformComponentManagerTest, FilledCachedInventory) {
    // Prepare test data
    PackageInventory packagesDiscovered;
    
        // Set up expectations on the mock object
    EXPECT_CALL(*mockEnv_.pkgUtil_, listPackages(_))
        .WillOnce(Return(expectedPackageList_));
    EXPECT_CALL(*mockEnv_.pkgUtil_, getPackageInfo(testPack1_, _))
        .WillOnce(Return(expectedPackageInfo_[0]));
    EXPECT_CALL(*mockEnv_.pkgUtil_, getPackageInfo(testPack2_, _))
        .WillRepeatedly(Return(expectedPackageInfo_[0]));
    
        // Invoke the function under test
    int32_t result = manager_->GetInstalledPackages(catalogRules_, packagesDiscovered);
    
    PackageInventory cachedInventory;
    int32_t resultCached = manager_->GetCachedInventory(cachedInventory);
    
    // Verify the result and expectations
    EXPECT_EQ(resultCached, 0);
    ASSERT_EQ(cachedInventory.packages.size(), 1);
    EXPECT_EQ(cachedInventory.packages[0].product, expectedPackageInfo_[0].packageIdentifier);
    EXPECT_EQ(cachedInventory.packages[0].version, expectedPackageInfo_[0].version);
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

// Test case for InstallComponent via Update
TEST_F(PmPlatformComponentManagerTest, UpdateComponentRebootSR) {
    // Prepare test data
    const std::string volumePath = "/Volumes/MountedVolume";
    PmComponent package;
    package.downloadedInstallerPath = "/path/to/package.pkg";
    package.signerName = "TestSigner";
    package.installerType = "pkg";
    package.installerArgs = "SR=true";
    
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
    ASSERT_EQ(manager_->UpdateComponent(package, errOut).pmResult, IPmPlatformComponentManager::PM_INSTALL_SUCCESS_REBOOT_REQUIRED);
}

// Test case for InstallComponent via Update
TEST_F(PmPlatformComponentManagerTest, UpdateComponentRebootFR) {
    // Prepare test data
    const std::string volumePath = "/Volumes/MountedVolume";
    PmComponent package;
    package.downloadedInstallerPath = "/path/to/package.pkg";
    package.signerName = "TestSigner";
    package.installerType = "pkg";
    package.installerArgs = "FR=true";
    
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
    ASSERT_EQ(manager_->UpdateComponent(package, errOut).pmResult, IPmPlatformComponentManager::PM_INSTALL_SUCCESS_REBOOT_REQUIRED);
}

// Test case for UninstallComponent with Pkg uninstaller
TEST_F(PmPlatformComponentManagerTest, UninstallComponentPkg) {
    // Prepare test data
    const std::string volumePath = "/Volumes/MountedVolume";
    PmComponent package;
    package.uninstallerLocation = "/path/to/package.pkg";
    package.uninstallerSignerName = "TestSigner";
    package.installerType = "pkg";

    // Set up expectations on the mock object
    EXPECT_CALL(*mockEnv_.pkgUtil_,
                installPackage(
                   package.uninstallerLocation.u8string(),
                   _, _
                ))
        .WillOnce(Return(true));
    
    // Set up expectations on the mock object
    EXPECT_CALL(*mockEnv_.fileUtils_,
                PathIsValid(
                   package.uninstallerLocation
                ))
        .WillOnce(Return(true));

    // Set up expectations on the codesignVerifier
    EXPECT_CALL(*mockCodesignVerifier_,
                PackageVerify(
                    package.uninstallerLocation,
                    package.uninstallerSignerName)
                )
        .WillOnce(Return(CodeSignStatus::CODE_SIGN_OK));

        // Invoke the function under test
    EXPECT_THAT(manager_->UninstallComponent(package), 0);
}

// Test case for UninstallComponent with .shuninstaller
TEST_F(PmPlatformComponentManagerTest, UninstallComponentSh) {
    // Prepare test data
    const std::string volumePath = "/Volumes/MountedVolume";
    PmComponent package;
    package.uninstallerLocation = "/path/to/package.sh";
    package.uninstallerSignerName = "";

    // Set up expectations on the mock object
    EXPECT_CALL(*mockEnv_.fileUtils_,
                PathIsValid( package.uninstallerLocation ))
        .WillOnce(Return(true));

    EXPECT_CALL(*mockEnv_.pkgUtil_,
                invokeShell( _, _ ))
        .WillOnce(Return(true));
    // Set up expectations on the codesignVerifier
    EXPECT_CALL(*mockCodesignVerifier_,
                PackageVerify( _, _) )
        .Times(0);

        // Invoke the function under test
    EXPECT_THAT(manager_->UninstallComponent(package), 0);
}
