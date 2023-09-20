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
        std::ofstream out_file( TEST_CONFIG_FILEPATH, std::ios::out );
        out_file << testdata.data;
        out_file.close();
    }
};

TEST( Config, Config_NoConfig )
{
    ConfigShared::Config config(&CMLogger::getInstance().getConfigLogger());
    EXPECT_EQ( ConfigShared::log::kDefaultLevel, config.getLogLevel() );
}

INSTANTIATE_TEST_SUITE_P( Config,
                          ConfigTest,
                          ::testing::Values( ConfigData{"{\"id\": {\"enable_verbose_logs\": true},\"pm\": {\"loglevel\": 7,\"CheckinInterval\": 300000,\"MaxStartupDelay\": 2000,\"maxFileCacheAge_s\": 604800,\"AllowPostInstallReboots\": true},\"uc\": {\"loglevel\": 7}}",(int)CM_LOG_LVL_T::CM_LOG_DEBUG,"DEBUG"},
                                             ConfigData{"{\"id\": {\"enable_verbose_logs\": true},\"pm\": {\"loglevel\": 7,\"CheckinInterval\": 300000,\"MaxStartupDelay\": 2000,\"maxFileCacheAge_s\": 604800,\"AllowPostInstallReboots\": true},\"uc\": {\"loglevel\": 5}}",(int)CM_LOG_LVL_T::CM_LOG_NOTICE,"NOTICE"},
                                             ConfigData{"{\"id\": {\"enable_verbose_logs\": true},\"pm\": {\"loglevel\": 7,\"CheckinInterval\": 300000,\"MaxStartupDelay\": 2000,\"maxFileCacheAge_s\": 604800,\"AllowPostInstallReboots\": true},\"uc\": {\"loglevel\": 10}}",ConfigShared::log::kDefaultLevel,"DEFAULT"},
                                             ConfigData{"{\"id\": {\"enable_verbose_logs\": true},\"uc\": {\"log_level\": 3}}",ConfigShared::log::kDefaultLevel,"DEFAULT"},
                                             ConfigData{"{\"uc\": {\"loglevel\": 5}}",(int)CM_LOG_LVL_T::CM_LOG_NOTICE,"NOTICE"},
                                             ConfigData{"{\"id\": {\"enable_verbose_logs\": true},\"loglevel\": 3}",ConfigShared::log::kDefaultLevel,"DEFAULT"},
                                             ConfigData{"{}",ConfigShared::log::kDefaultLevel,"DEFAULT"},
                                             ConfigData{"{\"uc\": {\"loglevel\":}}",ConfigShared::log::kDefaultLevel,"DEFAULT"},
                                             ConfigData{"",ConfigShared::log::kDefaultLevel,"DEFAULT"},
                                             ConfigData{"[\"test\":123]",ConfigShared::log::kDefaultLevel,"DEFAULT"},
                                             ConfigData{"{\"loglevel\":4}",ConfigShared::log::kDefaultLevel,"DEFAULT"} ) );

TEST_P( ConfigTest, getCmLogLevel )
{
    Config cfg(TEST_CONFIG_FILEPATH, &CMLogger::getInstance().getConfigLogger());
    cfg.subscribeForConfigChanges()();
    EXPECT_EQ( testdata.expectedLogLevel, cfg.getLogLevel() );
}

// Define a test case for Crashpad settings parsing
TEST(Config, ParseCrashpadSettings) {
    // Define a JSON string containing Crashpad settings
    const std::string crashpadSettings = R"(
        {
            "crashpad": {
                "pruneAge": 3,
                "pruneDbSize": 50,
                "uploadUrl": "bla-bla-bla"
            }
        }
    )";
    
    // Create a temporary configuration file with the Crashpad settings
    std::ofstream config_file(TEST_CONFIG_FILEPATH);
    config_file << crashpadSettings;
    config_file.close();

    // Create a Config object
    ConfigShared::Config config(TEST_CONFIG_FILEPATH, &CMLogger::getInstance().getConfigLogger());
    
    // Check if the parsed Crashpad settings match the expected values
    EXPECT_EQ(config.getCrashpadConfig().pruneAge, 3);
    EXPECT_EQ(config.getCrashpadConfig().pruneDbSize, 50);
    EXPECT_EQ(config.getCrashpadConfig().uploadUrl, "bla-bla-bla");
}
