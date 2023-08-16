/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#include "gtest/gtest.h"
#include "UnitTestBase.h"
#include "common/macOS/FileWatcher.hpp"
#include <fstream>

using testing::StrictMock;
using testing::Return;
using testing::_;

class WatchFileChangesTest: public TestEnv::UnitTestBase
{
public:
    WatchFileChangesTest()
    : watcher_("WatchFileChangesTest")
    {
        tempFilePath_ = generateTemporaryFileName();
        std::ofstream file(tempFilePath_);
        file.close();

    }
    ~WatchFileChangesTest()
    {
        std::filesystem::remove(tempFilePath_);
    }

protected:
    std::string generateTemporaryFileName()
    {
        std::string tempDir = std::filesystem::temp_directory_path();
        std::string tempFilePath;
        
        std::time_t now = std::time(nullptr);
        std::string timestamp = std::to_string(now);
        
        tempFilePath = tempDir + "/tempfile_" + timestamp;
        
        return tempFilePath;
    }

    FileWatcher watcher_;
    std::string tempFilePath_;
};

void aFunc(void)
{
}

TEST_F(WatchFileChangesTest, Add)
{
    ASSERT_THAT(watcher_.add(tempFilePath_, [=]() {aFunc();}), ::testing::IsTrue());
}

TEST_F(WatchFileChangesTest, DoubleAdd)
{
    ASSERT_THAT(watcher_.add(tempFilePath_, [=]() {aFunc();}), ::testing::IsTrue());
    ASSERT_THAT(watcher_.add(tempFilePath_, [=]() {aFunc();}), ::testing::IsFalse());
}

TEST_F(WatchFileChangesTest, EmptyAdd)
{
    ASSERT_THAT(watcher_.add("", [=]() {aFunc();}), ::testing::IsFalse());
}

TEST_F(WatchFileChangesTest, NoFunction)
{
    ASSERT_THAT(watcher_.add(tempFilePath_, {}), ::testing::IsFalse());
}

TEST_F(WatchFileChangesTest, Remove)
{
    ASSERT_THAT(watcher_.add(tempFilePath_, [=]() {aFunc();}), ::testing::IsTrue());
    ASSERT_THAT(watcher_.remove(tempFilePath_), ::testing::IsTrue());
}

TEST_F(WatchFileChangesTest, DoubleRemove)
{
    ASSERT_THAT(watcher_.add(tempFilePath_, [=]() {aFunc();}), ::testing::IsTrue());
    ASSERT_THAT(watcher_.remove(tempFilePath_), ::testing::IsTrue());
    ASSERT_THAT(watcher_.remove(tempFilePath_), ::testing::IsFalse());
}

TEST_F(WatchFileChangesTest, SingleRemoveWithoutAdd)
{
    ASSERT_THAT(watcher_.remove(tempFilePath_), ::testing::IsFalse());
}
