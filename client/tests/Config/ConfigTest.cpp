#include "gtest/gtest.h"
#include "Config.hpp"
#include "Logger/CMLogger.hpp"
#include <fstream>

using namespace ConfigShared;

#define TEST_CONFIG_FILEPATH "cm_config.json"
#define TEST_LOG_FILEPATH "./cm_test.log"

int TestLogGlobalInit()
{
    CM_LOG_INIT(TEST_LOG_FILEPATH);
    return 0;
}
// initialise CMLogger as the first thing
int i = TestLogGlobalInit();

struct ConfigData
{
    std::string data;
    int expectedLogLevel;
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
        std::remove( TEST_CONFIG_FILEPATH );
        testdata = GetParam();
        std::ofstream out_file( ConfigShared::Config::cmConfigPath + "/" + TEST_CONFIG_FILEPATH, std::ios::out );
        out_file << testdata.data;
        out_file.close();
    }
};

TEST( Config, Config_NoConfig )
{
    ConfigShared::Config config("uc", &CMLogger::getInstance().getConfigLogger());
    EXPECT_EQ( DEFAULT_LOG_LEVEL, config.getLogLevel() );
}

INSTANTIATE_TEST_SUITE_P( Config,
                          ConfigTest,
                          ::testing::Values( ConfigData{"{\"id\": {\"enable_verbose_logs\": true},\"pm\": {\"loglevel\": 7,\"CheckinInterval\": 300000,\"MaxStartupDelay\": 2000,\"maxFileCacheAge_s\": 604800,\"AllowPostInstallReboots\": true},\"uc\": {\"loglevel\": 7}}",(int)CM_LOG_LVL_T::CM_LOG_DEBUG,"DEBUG"},
                                             ConfigData{"{\"id\": {\"enable_verbose_logs\": true},\"pm\": {\"loglevel\": 7,\"CheckinInterval\": 300000,\"MaxStartupDelay\": 2000,\"maxFileCacheAge_s\": 604800,\"AllowPostInstallReboots\": true},\"uc\": {\"loglevel\": 5}}",(int)CM_LOG_LVL_T::CM_LOG_NOTICE,"NOTICE"},
                                             ConfigData{"{\"id\": {\"enable_verbose_logs\": true},\"pm\": {\"loglevel\": 7,\"CheckinInterval\": 300000,\"MaxStartupDelay\": 2000,\"maxFileCacheAge_s\": 604800,\"AllowPostInstallReboots\": true},\"uc\": {\"loglevel\": 10}}",DEFAULT_LOG_LEVEL,"DEFAULT"},
                                             ConfigData{"{\"id\": {\"enable_verbose_logs\": true},\"uc\": {\"log_level\": 3}}",DEFAULT_LOG_LEVEL,"DEFAULT"},
                                             ConfigData{"{\"uc\": {\"loglevel\": 5}}",(int)CM_LOG_LVL_T::CM_LOG_NOTICE,"NOTICE"},
                                             ConfigData{"{\"id\": {\"enable_verbose_logs\": true},\"loglevel\": 3}",DEFAULT_LOG_LEVEL,"DEFAULT"},
                                             ConfigData{"{}",DEFAULT_LOG_LEVEL,"DEFAULT"},
                                             ConfigData{"{\"uc\": {\"loglevel\":}}",DEFAULT_LOG_LEVEL,"DEFAULT"},
                                             ConfigData{"",DEFAULT_LOG_LEVEL,"DEFAULT"},
                                             ConfigData{"[\"test\":123]",DEFAULT_LOG_LEVEL,"DEFAULT"},
                                             ConfigData{"{\"loglevel\":4}",DEFAULT_LOG_LEVEL,"DEFAULT"} ) );

TEST_P( ConfigTest, getCmLogLevel )
{
    Config cfg(ConfigShared::Config::cmConfigPath + "/" + TEST_CONFIG_FILEPATH, "uc", &CMLogger::getInstance().getConfigLogger());
    cfg.subscribeForConfigChanges()();
    EXPECT_EQ( testdata.expectedLogLevel, cfg.getLogLevel() );
}
