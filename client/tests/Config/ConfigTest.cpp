#include "gtest/gtest.h"
#include "Configuration/Config.hpp"
#include <fstream>

using namespace CloudManagement;
#define TEST_CONFIG_FILEPATH "cm_config.json"

struct ConfigData
{
    std::string data;
    CM_LOG_LVL_T expectedLogLevel;
    std::string logLevelString;
};

static std::ostream&
operator<<( std::ostream& os, const ConfigData& sData )
{
    return os << "JSON Config: " << sData.data << "; Expected Log Level: " << sData.logLevelString;
}

class ConfigTest : public ::testing::TestWithParam<ConfigData>
{
public:
    ConfigData testdata;
    ConfigTest()
    {
        testdata = GetParam();
        std::ofstream out_file( TEST_CONFIG_FILEPATH, std::ios::out );
        out_file << testdata.data;
        out_file.close();
    }

    static void TearDownTestSuite()
    {
       std::remove( TEST_CONFIG_FILEPATH );
    }
};

TEST( Config, Config_NoConfig )
{
    Config config;
    EXPECT_EQ( DEFAULT_LOG_LEVEL, config.getLogLevel() );
}

INSTANTIATE_TEST_SUITE_P( Config,
                          ConfigTest,
                          ::testing::Values( ConfigData{"{\"id\": {\"enable_verbose_logs\": true},\"pm\": {\"loglevel\": 7,\"CheckinInterval\": 300000,\"MaxStartupDelay\": 2000,\"maxFileCacheAge_s\": 604800,\"AllowPostInstallReboots\": true},\"uc\": {\"loglevel\": 7}}",CM_LOG_LVL_T::CM_LOG_DEBUG,"DEBUG"},
                                             ConfigData{"{\"id\": {\"enable_verbose_logs\": true},\"pm\": {\"loglevel\": 7,\"CheckinInterval\": 300000,\"MaxStartupDelay\": 2000,\"maxFileCacheAge_s\": 604800,\"AllowPostInstallReboots\": true},\"uc\": {\"loglevel\": 5}}",CM_LOG_LVL_T::CM_LOG_NOTICE,"NOTICE"},
                                             ConfigData{"{\"id\": {\"enable_verbose_logs\": true},\"pm\": {\"loglevel\": 7,\"CheckinInterval\": 300000,\"MaxStartupDelay\": 2000,\"maxFileCacheAge_s\": 604800,\"AllowPostInstallReboots\": true},\"uc\": {\"loglevel\": 10}}",DEFAULT_LOG_LEVEL,"DEAFULT"},
                                             ConfigData{"{\"id\": {\"enable_verbose_logs\": true},\"uc\": {\"log_level\": 3}}",DEFAULT_LOG_LEVEL,"DEAFULT"},
                                             ConfigData{"{\"uc\": {\"loglevel\": 5}}",CM_LOG_LVL_T::CM_LOG_NOTICE,"NOTICE"},
                                             ConfigData{"{\"id\": {\"enable_verbose_logs\": true},\"loglevel\": 3}",DEFAULT_LOG_LEVEL,"DEAFULT"},
                                             ConfigData{"{}",DEFAULT_LOG_LEVEL,"DEAFULT"},
                                             ConfigData{"{\"uc\": {\"loglevel\":}}",DEFAULT_LOG_LEVEL,"DEAFULT"},
                                             ConfigData{"",DEFAULT_LOG_LEVEL,"DEAFULT"},
                                             ConfigData{"[\"test\":123]",DEFAULT_LOG_LEVEL,"DEAFULT"},
                                             ConfigData{"{\"loglevel\":4}",DEFAULT_LOG_LEVEL,"DEAFULT"} ) );

TEST_P( ConfigTest, getLogLevel )
{
    Config cfg;
    EXPECT_EQ( testdata.expectedLogLevel, cfg.getLogLevel() );
}
