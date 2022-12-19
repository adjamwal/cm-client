#include "gtest/gtest.h"
#include "Logger/CMLogger.hpp"
#include <fstream>
#include <thread>

#define TEST_LOG_FILE "csc_cms_test.log"
#define TEST_LOG_FILEPATH "./csc_cms_test.log"

// Below tests are intended to be executed in the same order. Do not shuffle
TEST( CMLogger, getInstance_EmptyLogname )
{
    EXPECT_THROW( CMLogger::getInstance( "" ), CMLogger::logger_exception );
}

TEST( CMLogger, getInstance_createLogFileFailure )
{
    EXPECT_THROW( CMLogger::getInstance( TEST_LOG_FILE ), CMLogger::logger_exception );
}

class CMLoggerTest : public ::testing::Test
{
public:

    void SetUp() override
    {
        ASSERT_NO_THROW( CMLogger::getInstance( TEST_LOG_FILEPATH ) );
    }
};

TEST_F( CMLoggerTest, setLogLevel )
{
    CMLogger::getInstance().setLogLevel( CM_LOG_LVL_T::CM_LOG_ALERT );
    CM_LOG_DEBUG( "DEBUG" );
    CM_LOG_ALERT( "ALERT" );
    std::string logString, line;
    std::ifstream in_file( TEST_LOG_FILEPATH, std::ios::in );
    if ( !in_file.is_open() ) {
        FAIL();
    }
    while ( getline( in_file, line ) ) {
        logString.append( line );
    }  
    in_file.close();
    EXPECT_NE( std::string::npos,logString.find( "ALERT" ) );
    EXPECT_EQ( std::string::npos,logString.find( "DEBUG" ) );
}

TEST_F( CMLoggerTest, logFromMultipleThreads )
{
    CMLogger::getInstance().setLogLevel( CM_LOG_LVL_T::CM_LOG_ERROR );
    std::thread thread1([]() {
        CM_LOG_ERROR( "HELLO" );
        });
    std::thread thread2([]() {
        CM_LOG_ERROR( "HELLO" );
        });
    std::thread thread3([]() {
        CM_LOG_ERROR( "HELLO" );
        });
    std::thread thread4([]() {
        CM_LOG_ERROR( "HELLO" );
        });
    std::thread thread5([]() {
        CM_LOG_ERROR( "HELLO" );
        });
    std::thread thread6([]() {
        CM_LOG_ERROR( "HELLO" );
        });

    thread1.join();
    thread2.join();
    thread3.join();
    thread4.join();
    thread5.join();
    thread6.join();


    std::string logString, line;
    std::ifstream in_file( TEST_LOG_FILEPATH, std::ios::in );
    if ( !in_file.is_open() ) {
        FAIL();
    }
    while ( getline( in_file, line  )) {
        logString.append( line );
    }  
    in_file.close();
    size_t idx = logString.find( "HELLO" );
    unsigned occurances = 0;
    while ( idx != std::string::npos )
    {
        idx = logString.find( "HELLO", idx + 1 );
        occurances++;
    }

    EXPECT_EQ( occurances, 6 );
}

TEST_F( CMLoggerTest, logRotate )
{
    EXPECT_TRUE( CMLogger::getInstance().setLogConfig( 70,3 ) );
    // The three lines below are 99 bytes each in the file
    CM_LOG_ERROR( "This is a test" );
    CM_LOG_ERROR( "This is a test" );
    CM_LOG_ERROR( "This is a test" );

    std::string tempPathName = TEST_LOG_FILEPATH;
    tempPathName = tempPathName.substr( 0, tempPathName.length() - 4 ); // strip off .log
    
    std::filesystem::path logFilePath = std::filesystem::path( "./" );
    std::filesystem::path pa;
    unsigned occurances = 0;
    try
    {
        for ( auto const& dirEntry : std::filesystem::directory_iterator{logFilePath} ) {
            if ( std::filesystem::exists( dirEntry ) && !std::filesystem::is_directory( dirEntry ) ) {
                pa = dirEntry.path();
                if( std::filesystem::is_regular_file( pa ) ) {
                    if( std::string::npos != pa.string().find( tempPathName ) ) {
                        occurances++;
                    }
                }
            }
        }
    }
    catch( std::filesystem::filesystem_error& e )
    {
      std::cout << e.what() << "\n";
    }
    EXPECT_EQ( occurances, 3 );
}
