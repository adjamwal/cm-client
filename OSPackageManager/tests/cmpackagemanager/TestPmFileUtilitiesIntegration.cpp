#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <fstream>
#include "FileUtilities.hpp"

// Helper function to check if a file has read permission for others
bool CheckFileHasReadPermission(const std::filesystem::path &filePath)
{
    const auto pathPermissions = std::filesystem::status(filePath).permissions();
    return (std::filesystem::perms::others_read == (pathPermissions & std::filesystem::perms::others_read));
}

class FileUtilitiesIntegrationTest : public ::testing::Test {
protected:
    std::string tempFileName_;
    
    void SetUp() override {
        // Create a unique temporary file in the system's temporary directory
        char tempFileNameBuffer[L_tmpnam];
        if (tmpnam(tempFileNameBuffer) == nullptr) {
            FAIL() << "Failed to generate a temporary file name.";
        }
        tempFileName_ = std::filesystem::temp_directory_path() / tempFileNameBuffer;
        
        // Ensure the file exists before the test
        std::ofstream file(tempFileName_);
        file.close();
    }
    
    void TearDown() override {
        // Clean up the temporary file after the test
        std::filesystem::remove(tempFileName_);
    }
};

TEST_F(FileUtilitiesIntegrationTest, ApplyUserRestrictions)
{
    std::error_code errCode;

    ASSERT_THAT(std::filesystem::exists(tempFileName_), ::testing::IsTrue());
    
    // remove read property completely
    std::filesystem::permissions(tempFileName_,
                                 std::filesystem::perms::others_read,
                                 std::filesystem::perm_options::remove, errCode);
    
    // Check if the file already has read permission for others before applying restrictions
    ASSERT_THAT(CheckFileHasReadPermission(tempFileName_), ::testing::IsFalse());
    
    // Apply user restrictions to the file
    PackageManager::FileUtilities fu;
    ASSERT_THAT(fu.ApplyUserRestrictions(tempFileName_), ::testing::IsTrue());
    
        // Check if the file has read permission for others after applying restrictions
    ASSERT_THAT(CheckFileHasReadPermission(tempFileName_), ::testing::IsTrue());

}

MATCHER_P(ContainsPathChild, childPath, "Checks if path argument contains childPath"){
    for (const auto& path : arg) {
         if (std::string::npos != path.string().find(childPath)) {
             return true;
         }
     }
     
     //Preparing the error output
     std::string finalPath;
     for (const auto& path : arg) {
         finalPath += ( path.string() + "\n");
     }
     
    *result_listener << "Argument: " << childPath<< " was not found in \n" << finalPath << "\n list";
    return false;
}

// Test case for FileSearchWithWildCard
TEST(FileUtilities, FileSearchWithWildCardTestWithAsterisk) {
    std::vector<std::filesystem::path> results;
    
    const std::filesystem::path searchPath = "root/*/*";
    
    // Call the function under test
    PackageManager::FileUtilities fileUtils;
    
    // Check the results
    EXPECT_EQ(fileUtils.FileSearchWithWildCard(searchPath, results), 0); // Assuming 0 means success
    EXPECT_THAT(results, ::testing::SizeIs(3));
    // Use the custom matcher
    EXPECT_THAT(results, ContainsPathChild("signedprocess"));
    EXPECT_THAT(results, ContainsPathChild("child1"));
    EXPECT_THAT(results, ContainsPathChild("child2"));
}

TEST(FileUtilities, FileSearchWithWildCardTest1) {
    std::vector<std::filesystem::path> results;
    
    const std::filesystem::path searchPath = "root/child?/*";
    
        // Call the function under test
    PackageManager::FileUtilities fileUtils;
    
    // Check the results
    EXPECT_EQ(fileUtils.FileSearchWithWildCard(searchPath, results), 0); // Assuming 0 means success
    EXPECT_THAT(results, ::testing::SizeIs(3));
    // Use the custom matcher
    EXPECT_THAT(results, ContainsPathChild("signedprocess"));
    EXPECT_THAT(results, ContainsPathChild("child1"));
    EXPECT_THAT(results, ContainsPathChild("child2"));
}

TEST(FileUtilities, FileSearchWithWildCardTest2) {
    std::vector<std::filesystem::path> results;
    
    const std::filesystem::path searchPath = "root/child?/subchild?";
    
    // Call the function under test
    PackageManager::FileUtilities fileUtils;
    
    // Check the results
    EXPECT_EQ(fileUtils.FileSearchWithWildCard(searchPath, results), 0); // Assuming 0 means success
    EXPECT_THAT(results, ::testing::SizeIs(2));
    // Use the custom matcher
    EXPECT_THAT(results, ContainsPathChild("subchild1"));
    EXPECT_THAT(results, ContainsPathChild("subchild2"));
}

TEST(FileUtilities, FileSearchWithWildCardTest3) {
    std::vector<std::filesystem::path> results;
    
    const std::filesystem::path searchPath = "root/child?/*child?";
    
    // Call the function under test
    PackageManager::FileUtilities fileUtils;
    
    // Check the results
    EXPECT_EQ(fileUtils.FileSearchWithWildCard(searchPath, results), 0); // Assuming 0 means success
    EXPECT_THAT(results, ::testing::SizeIs(2));
    // Use the custom matcher
    EXPECT_THAT(results, ContainsPathChild("subchild1"));
    EXPECT_THAT(results, ContainsPathChild("subchild2"));
}

TEST(FileUtilities, FileSearchWithWildCardTest4) {
    std::vector<std::filesystem::path> results;
    
    const std::filesystem::path searchPath = "root/*/s*";
    
    // Call the function under test
    PackageManager::FileUtilities fileUtils;
    
    // Check the results
    EXPECT_EQ(fileUtils.FileSearchWithWildCard(searchPath, results), 0); // Assuming 0 means success
    EXPECT_THAT(results, ::testing::SizeIs(3));
    // Use the custom matcher
    EXPECT_THAT(results, ContainsPathChild("signedprocess"));
    EXPECT_THAT(results, ContainsPathChild("subchild1"));
    EXPECT_THAT(results, ContainsPathChild("subchild2"));
}

TEST(FileUtilities, FileSearchWithWildCardTest5) {
    std::vector<std::filesystem::path> results;
    
    const std::filesystem::path searchPath = "r*/*/*";
    
        // Call the function under test
    PackageManager::FileUtilities fileUtils;
    
    // Check the results
    EXPECT_EQ(fileUtils.FileSearchWithWildCard(searchPath, results), 0); // Assuming 0 means success
    EXPECT_THAT(results, ::testing::SizeIs(3));
    // Use the custom matcher
    EXPECT_THAT(results, ContainsPathChild("signedprocess"));
    EXPECT_THAT(results, ContainsPathChild("child1"));
    EXPECT_THAT(results, ContainsPathChild("child2"));
    EXPECT_THAT(results, ContainsPathChild("subchild1"));
    EXPECT_THAT(results, ContainsPathChild("subchild2"));
}

