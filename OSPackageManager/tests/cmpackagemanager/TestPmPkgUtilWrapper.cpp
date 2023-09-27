#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "PmPkgUtilWrapper.hpp"
#include "UnitTestBase.h"

using ::testing::NiceMock;
using ::testing::ElementsAreArray;
using ::testing::SizeIs;

class MockPkgUtilWrapper : public PmPkgUtilWrapper {
public:
    MOCK_METHOD(std::string, executeCommand, (const std::string& command), (const, override));
};

// Fixture for PkgUtilWrapper tests
class PkgUtilWrapperTest : public TestEnv::UnitTestBase {
protected:
    NiceMock<MockPkgUtilWrapper> mockWrapper;
};

TEST_F(PkgUtilWrapperTest, ListPackagesTest) {
    std::vector<std::string> expectedPackages = {"package1", "package2", "package3"};
    std::string mockOutput = "package1\npackage2\npackage3\n";
    
    EXPECT_CALL(mockWrapper, executeCommand("/usr/sbin/pkgutil --packages"))
        .WillOnce(
            ::testing::Return(mockOutput)
        );
    
    EXPECT_THAT(mockWrapper.listPackages(), ElementsAreArray(expectedPackages));
}

TEST_F(PkgUtilWrapperTest, GetPackageInfoTest) {
    std::string packageIdentifier = "com.example.package";
    std::string mockOutput = "package-id: com.example.package\nversion: 1.0\nlocation: /path/to/package\n";
    
    EXPECT_CALL(mockWrapper, executeCommand("/usr/sbin/pkgutil --pkg-info com.example.package"))
        .WillOnce(
            ::testing::Return(mockOutput)
        );
    
    PmPackageInfo packageInfo = mockWrapper.getPackageInfo(packageIdentifier, "");
    ASSERT_EQ(packageInfo.packageIdentifier, "com.example.package");
    ASSERT_EQ(packageInfo.version, "1.0");
    ASSERT_EQ(packageInfo.installationPath, "/path/to/package");
}

TEST_F(PkgUtilWrapperTest, ListPackageFilesTest) {
    std::vector<std::string> expectedFiles = {"file1", "file2", "file3"};
    std::string packageIdentifier = "com.example.package";
    std::string mockOutput = "file1\nfile2\nfile3\n";
    
    EXPECT_CALL(mockWrapper, executeCommand("/usr/sbin/pkgutil --files com.example.package"))
        .WillOnce(
             ::testing::Return(mockOutput)
         );
    
    EXPECT_THAT(mockWrapper.listPackageFiles(packageIdentifier), ElementsAreArray(expectedFiles));
}

// Test case for successful package installation
TEST_F(PkgUtilWrapperTest, InstallPackage_Success) {
    const std::string packagePath = "/path/to/package.pkg";
    const std::string volumePath = "/Volumes/MountedVolume";
    
    // Define the expected command and output
    const std::string expectedCommand = "/usr/sbin/installer -pkg /path/to/package.pkg -target /Volumes/MountedVolume";
    const std::string expectedOutput = "The install was successful.";
    
    // Set up the mock behavior
    EXPECT_CALL(mockWrapper, executeCommand(expectedCommand)).WillOnce(::testing::Return(expectedOutput));
    
    // Perform the installation
    EXPECT_THAT(mockWrapper.installPackage(packagePath, {}, volumePath), ::testing::IsTrue());
}

// Test case for successful package installation
TEST_F(PkgUtilWrapperTest, UpgradePackage_Success) {
    const std::string packagePath = "/path/to/package.pkg";
    const std::string volumePath = "/Volumes/MountedVolume";

    // Define the expected command and output
    const std::string expectedCommand = "/usr/sbin/installer -pkg /path/to/package.pkg -target /Volumes/MountedVolume";
    const std::string expectedOutput = "The upgrade was successful.";

    // Set up the mock behavior
    EXPECT_CALL(mockWrapper, executeCommand(expectedCommand)).WillOnce(::testing::Return(expectedOutput));

    // Perform the installation
    EXPECT_THAT(mockWrapper.installPackage(packagePath, {}, volumePath), ::testing::IsTrue());
}

// Test case for failed package installation
TEST_F(PkgUtilWrapperTest, InstallPackage_Failure) {
    const std::string packagePath = "/path/to/package.pkg";
    const std::string volumePath = "/Volumes/MountedVolume";
    
    // Define the expected command and output
    const std::string expectedCommand = "/usr/sbin/installer -pkg /path/to/package.pkg -target /Volumes/MountedVolume";
    const std::string expectedOutput = "The install failed.";
    
    // Set up the mock behavior
    EXPECT_CALL(mockWrapper, executeCommand(expectedCommand)).WillOnce(::testing::Return(expectedOutput));
    
    // Perform the installation
    EXPECT_THAT(mockWrapper.installPackage(packagePath, {}, volumePath), ::testing::IsFalse());
}

// Test case for successful package uninstallation
TEST_F(PkgUtilWrapperTest, UninstallPackage_Success) {
    const std::string packageIdentifier = "com.example.package";
    
    // Define the expected command and output
    const std::string expectedCommand = "/usr/sbin/pkgutil --force --forget com.example.package";
    const std::string expectedOutput = "No receipt found.";
    
    // Set up the mock behavior
    EXPECT_CALL(mockWrapper, executeCommand(expectedCommand)).WillOnce(::testing::Return(expectedOutput));
    
    // Perform the uninstallation
    EXPECT_THAT( mockWrapper.uninstallPackage(packageIdentifier), ::testing::IsTrue());
}

// Test case for failed package uninstallation
TEST_F(PkgUtilWrapperTest, UninstallPackage_Failure) {
    const std::string packageIdentifier = "com.example.package";
    
    // Define the expected command and output
    const std::string expectedCommand = "/usr/sbin/pkgutil --force --forget com.example.package";
    const std::string expectedOutput = "Unable to forget package.";
    
    // Set up the mock behavior
    EXPECT_CALL(mockWrapper, executeCommand(expectedCommand)).WillOnce(::testing::Return(expectedOutput));
    
    // Perform the uninstallation
    EXPECT_THAT(mockWrapper.uninstallPackage(packageIdentifier), ::testing::IsFalse());
}
