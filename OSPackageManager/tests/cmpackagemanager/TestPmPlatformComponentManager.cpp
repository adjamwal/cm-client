#include <gtest/gtest.h>
#include "IPmPkgUtil.hpp"
#include "MockPmPlatformComponentManager/MockPmPkgUtil.hpp"
#include "PmPlatformComponentManager.hpp"
#include "PmLogger.hpp"

using namespace testing;

// Test fixture for PmPlatformComponentManager tests
class PmPlatformComponentManagerTest : public Test {
protected:
    void SetUp() override {
        PmLogger::initLogger();
        // Create a mock object for IPmPkgUtil
        mockPkgUtil_ = std::make_shared<MockPmPkgUtil>();
        
        // Initialize PmPlatformComponentManager with the mock object
        manager_ = std::shared_ptr<PmPlatformComponentManager>( new PmPlatformComponentManager(mockPkgUtil_));
    }
    
    void TearDown() override {
        // Reset the mock object
        mockPkgUtil_.reset();
        manager_.reset();
        PmLogger::releaseLogger();
    }
    
    // Mocked IPmPkgUtil object
    std::shared_ptr<MockPmPkgUtil> mockPkgUtil_;
    
    // Instance of PmPlatformComponentManager to be tested
    std::shared_ptr<PmPlatformComponentManager> manager_;
};

// Test case for GetInstalledPackages
TEST_F(PmPlatformComponentManagerTest, GetInstalledPackages) {
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
    EXPECT_CALL(*mockPkgUtil_, listPackages(_))
        .WillOnce(Return(expectedPackageList));
    EXPECT_CALL(*mockPkgUtil_, getPackageInfo("com.test.Package1", _))
        .WillOnce(Return(expectedPackageInfo[0]));
    EXPECT_CALL(*mockPkgUtil_, getPackageInfo("com.test.Package2", _))
        .WillOnce(Return(expectedPackageInfo[1]));

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
    EXPECT_CALL(*mockPkgUtil_, listPackages(_))
        .WillOnce(Return(expectedPackageList));
    EXPECT_CALL(*mockPkgUtil_, getPackageInfo("com.test.Package1", _))
        .WillOnce(Return(expectedPackageInfo[0]));
    EXPECT_CALL(*mockPkgUtil_, getPackageInfo("com.test.Package2", _))
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
