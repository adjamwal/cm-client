#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <Foundation/Foundation.h>
#include <fstream>
#include <regex>
#include "FileUtilities.hpp"
#include "PmLogger.hpp"

// Helper function to check if a file has read permission for others
bool CheckFileHasReadPermission(const std::filesystem::path &filePath)
{
    const auto pathPermissions = std::filesystem::status(filePath).permissions();
    return (std::filesystem::perms::others_read == (pathPermissions & std::filesystem::perms::others_read));
}

class FileUtilitiesIntegrationTest : public ::testing::Test {
protected:
    std::string tempFileName_;
    
    std::string generateTemporaryFileName()
    {
        std::string tempDir = std::filesystem::temp_directory_path();
        std::string tempFileName;
        
            // You can generate a random string or use a timestamp to make the filename unique.
            // Here, I'll use a simple timestamp-based approach for demonstration purposes.
        std::time_t now = std::time(nullptr);
        std::string timestamp = std::to_string(now);
        
        tempFileName = tempDir + "/tempfile_" + timestamp;
        
        return tempFileName;
    }
    
    FileUtilitiesIntegrationTest() {
        tempFileName_ = generateTemporaryFileName();
        
        // Ensure the file exists before the test
        std::ofstream file(tempFileName_);
        file.close();
    }
    
    ~FileUtilitiesIntegrationTest() {
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

TEST(FileUtilities, ResolvePath)
{
    PmLogger::initLogger();
    
    const auto getKnownFolderPath = [](NSSearchPathDirectory directory) -> std::string {
        std::string knownFolder;
        NSArray* paths = NSSearchPathForDirectoriesInDomains(directory, NSSystemDomainMask, YES);
        if (paths.count > 0) {
            NSString* path = [paths objectAtIndex:0];
            knownFolder = [path UTF8String];
        }
        return knownFolder;
    };
    
    const std::unordered_map<std::string, unsigned int> knownFolderMap = {
        { "ApplicationDirectory", NSApplicationDirectory },
        { "AdminApplicationDirectory", NSAdminApplicationDirectory },
        { "LibraryDirectory", NSLibraryDirectory },
        { "DeveloperDirectory", NSDeveloperDirectory },
        { "DocumentationDirectory", NSDocumentationDirectory },
        { "CoreServiceDirectory", NSCoreServiceDirectory },
        { "CachesDirectory", NSCachesDirectory },
        { "ApplicationSupportDirectory", NSApplicationSupportDirectory },
        { "InputMethodsDirectory", NSInputMethodsDirectory },
        { "PrinterDescriptionDirectory", NSPrinterDescriptionDirectory },
        { "AllApplicationsDirectory", NSAllApplicationsDirectory },
    };

    PackageManager::FileUtilities fu;
    const std::string pathTemplate = "<FOLDERID_%s>/path2";
    
    std::vector<std::string> resolvedPaths;
    std::vector<std::string> expectedPaths;
    
    for (const auto& [folderID, folderDirectory] : knownFolderMap) {
        
        const std::string basePath = std::regex_replace(pathTemplate, std::regex("%s"), folderID);

        const std::string knownFolder = getKnownFolderPath((NSSearchPathDirectory)folderDirectory);
        if (!knownFolder.empty()) {
            resolvedPaths.push_back( fu.ResolvePath(basePath) );

            std::string expectedPath = pathTemplate;
            const auto pos = expectedPath.find("<FOLDERID_%s>");
            if (pos != std::string::npos) {
                expectedPath.replace(pos, std::string("<FOLDERID_%s>").length(), knownFolder);
            }
            expectedPaths.push_back(expectedPath);
        } else {
            std::cout << "folder " << folderID << " is not supported" << std::endl;
        }
    }
    
EXPECT_THAT(expectedPaths, ::testing::ElementsAreArray(resolvedPaths));
    
    PmLogger::releaseLogger();
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
