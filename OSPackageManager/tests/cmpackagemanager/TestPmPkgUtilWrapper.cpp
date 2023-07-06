#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "PmPkgUtilWrapper.hpp"

class MockPkgUtilWrapper : public PmPkgUtilWrapper {
public:
    MOCK_METHOD(std::string, executeCommand, (const std::string& command, const std::string& volumePath), (const, override));
};

TEST(PkgUtilWrapperTest, ListPackagesTest) {
    MockPkgUtilWrapper pkgUtil;
    std::vector<std::string> expectedPackages = {"package1", "package2", "package3"};
    std::string mockOutput = "package1\npackage2\npackage3\n";
    
    EXPECT_CALL(pkgUtil, executeCommand("/usr/sbin/pkgutil --packages", ""))
        .WillOnce(
            ::testing::Return(mockOutput)
        );
    
    std::vector<std::string> packages = pkgUtil.listPackages();
    ASSERT_EQ(packages.size(), expectedPackages.size());
    for (size_t i = 0; i < packages.size(); ++i) {
        ASSERT_EQ(packages[i], expectedPackages[i]);
    }
}

TEST(PkgUtilWrapperTest, GetPackageInfoTest) {
    MockPkgUtilWrapper pkgUtil;
    std::string packageIdentifier = "com.example.package";
    std::string mockOutput = "package-id: com.example.package\nversion: 1.0\nlocation: /path/to/package\n";
    
    EXPECT_CALL(pkgUtil, executeCommand("/usr/sbin/pkgutil --pkg-info com.example.package", ""))
        .WillOnce(
            ::testing::Return(mockOutput)
        );
    
    PmPackageInfo packageInfo = pkgUtil.getPackageInfo(packageIdentifier, "");
    ASSERT_EQ(packageInfo.packageIdentifier, "com.example.package");
    ASSERT_EQ(packageInfo.version, "1.0");
    ASSERT_EQ(packageInfo.installationPath, "/path/to/package");
}

TEST(PkgUtilWrapperTest, ListPackageFilesTest) {
    MockPkgUtilWrapper pkgUtil;
    std::vector<std::string> expectedFiles = {"file1", "file2", "file3"};
    std::string packageIdentifier = "com.example.package";
    std::string mockOutput = "file1\nfile2\nfile3\n";
    
    EXPECT_CALL(pkgUtil, executeCommand("/usr/sbin/pkgutil --files com.example.package", ""))
        .WillOnce(
             ::testing::Return(mockOutput)
         );
    
    std::vector<std::string> files = pkgUtil.listPackageFiles(packageIdentifier);
    ASSERT_EQ(files.size(), expectedFiles.size());
    for (size_t i = 0; i < files.size(); ++i) {
        ASSERT_EQ(files[i], expectedFiles[i]);
    }
}
