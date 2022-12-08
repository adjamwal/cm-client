#include "gtest/gtest.h"
#include "Configuration/Config.hpp"
#include <fstream>

using namespace CloudManagementConfiguration;
#define TEST_CONFIG_FILEPATH "cm_config.json"

struct ConfigData
{
    std::string data;
    uint32_t expectedLogLevel;
};

static std::ostream&
operator<<(std::ostream& os, const ConfigData& sData)
{
    return os << "JSON Config: " << sData.data << "; Expected Log Level: " << sData.expectedLogLevel;
}

class ConfigTest : public ::testing::TestWithParam<ConfigData>
{
public:
    ConfigData testdata;
    ConfigTest()
    {
        testdata = GetParam();
        std::ofstream out_file(TEST_CONFIG_FILEPATH, std::ios::out);
        out_file << testdata.data;
        out_file.close();
    }

    static void TearDownTestSuite()
    {
       std::remove(TEST_CONFIG_FILEPATH);
    }
};

TEST(Config, Config_NoConfig)
{
    Config config;
    config.load();
    EXPECT_EQ(DEFAULT_LOG_LEVEL, config.getLogLevel());
}

INSTANTIATE_TEST_SUITE_P(Config,
                        ConfigTest,
                        ::testing::Values(ConfigData{"{\"id\": {\"enable_verbose_logs\": true},\"pm\": {\"loglevel\": 7,\"CheckinInterval\": 300000,\"MaxStartupDelay\": 2000,\"maxFileCacheAge_s\": 604800,\"AllowPostInstallReboots\": true},\"uc\": {\"loglevel\": 7}}",7},
                                          ConfigData{"{\"id\": {\"enable_verbose_logs\": true},\"pm\": {\"loglevel\": 7,\"CheckinInterval\": 300000,\"MaxStartupDelay\": 2000,\"maxFileCacheAge_s\": 604800,\"AllowPostInstallReboots\": true},\"uc\": {\"loglevel\": 5}}",5},
                                          ConfigData{"{\"id\": {\"enable_verbose_logs\": true},\"pm\": {\"loglevel\": 7,\"CheckinInterval\": 300000,\"MaxStartupDelay\": 2000,\"maxFileCacheAge_s\": 604800,\"AllowPostInstallReboots\": true},\"uc\": {\"loglevel\": 10}}",DEFAULT_LOG_LEVEL},
                                          ConfigData{"{\"id\": {\"enable_verbose_logs\": true},\"uc\": {\"log_level\": 3}}",DEFAULT_LOG_LEVEL},
                                          ConfigData{"{\"uc\": {\"loglevel\": 5}}",5},
                        	              ConfigData{"{\"id\": {\"enable_verbose_logs\": true},\"loglevel\": 3}",DEFAULT_LOG_LEVEL},
                                          ConfigData{"{}",DEFAULT_LOG_LEVEL},
                                          ConfigData{"{\"uc\": {\"loglevel\":}}",DEFAULT_LOG_LEVEL},
                                          ConfigData{"",DEFAULT_LOG_LEVEL},
                                          ConfigData{"[\"test\":123]",DEFAULT_LOG_LEVEL},
                                          ConfigData{"{\"loglevel\":4}",DEFAULT_LOG_LEVEL}));

TEST_P(ConfigTest, getLogLevel)
{
    Config cfg;
    cfg.load();
    EXPECT_EQ(testdata.expectedLogLevel, cfg.getLogLevel());
}
