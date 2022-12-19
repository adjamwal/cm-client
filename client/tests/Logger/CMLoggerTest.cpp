#include "gtest/gtest.h"
#include "Logger/CMLogger.hpp"
#include <fstream>

#define TEST_LOG_FILE "csc_cms_test.log"
#define TEST_LOG_FILEPATH "./csc_cms_test.log"

TEST(CMLogger, getInstance_EmptyLogname)
{
    EXPECT_THROW(CMLogger::getInstance(""), CMLogger::logger_exception);
}

TEST(CMLogger, getInstance_createLogFileFailure)
{
    EXPECT_THROW(CMLogger::getInstance(TEST_LOG_FILE), CMLogger::logger_exception);
}

TEST(CMLogger, setLogConfig)
{
    ASSERT_NO_THROW(CMLogger::getInstance(TEST_LOG_FILEPATH));
    EXPECT_TRUE(CMLogger::getInstance().setLogConfig(5242,5));
    EXPECT_FALSE(CMLogger::getInstance().setLogConfig(0,5));
    std::remove(TEST_LOG_FILE);
}