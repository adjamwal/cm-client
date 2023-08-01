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
