#include "Config.hpp"
#include "PmLogger.hpp"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <memory>

using namespace ConfigShared;

namespace
{
constexpr std::string_view kLogFile = "027F3A4D_256A_4D7C_A593_C705289D03E0_config_file.json";

struct ConfigData
{
    std::string data;
    IPMLogger::Severity expectedLogLevel;
    std::string logLevelString;
};

ConfigData kConfigValues[] {
    ConfigData{"{"
        "\"id\": {"
        "  \"enable_verbose_logs\": true"
        "},"
        "\"pm\": {"
        "  \"loglevel\": 7,"
        "  \"CheckinInterval\": 300000,"
        "  \"MaxStartupDelay\": 2000,"
        "  \"maxFileCacheAge_s\": 604800,"
        "  \"AllowPostInstallReboots\": true"
        "},"
        "\"uc\": {"
        "  \"loglevel\": 7"
        "}"
        "}",IPMLogger::LOG_DEBUG,"DEBUG"},
    ConfigData{"{"
        "\"id\": {"
        "  \"enable_verbose_logs\": true"
        "},"
        "\"pm\": {"
        "  \"loglevel\": 6,"
        "  \"CheckinInterval\": 300000,"
        "  \"MaxStartupDelay\": 2000,"
        "  \"maxFileCacheAge_s\": 604800,"
        "  \"AllowPostInstallReboots\": true"
        "},"
        "\"uc\": {"
        "  \"loglevel\": 7"
        "}"
        "}",IPMLogger::LOG_INFO,"INFO"},
    ConfigData{"{"
        "\"id\": {"
        "  \"enable_verbose_logs\": true"
        "},"
        "\"pm\": {"
        "  \"loglevel\": 5,"
        "  \"CheckinInterval\": 300000,"
        "  \"MaxStartupDelay\": 2000,"
        "  \"maxFileCacheAge_s\": 604800,"
        "  \"AllowPostInstallReboots\": true"
        "},"
        "\"uc\": {"
        "  \"loglevel\": 7"
        "}"
        "}",IPMLogger::LOG_NOTICE,"NOTICE"},
    ConfigData{"{"
        "\"id\": {"
        "  \"enable_verbose_logs\": true"
        "},"
        "\"pm\": {"
        "  \"loglevel\": 4,"
        "  \"CheckinInterval\": 300000,"
        "  \"MaxStartupDelay\": 2000,"
        "  \"maxFileCacheAge_s\": 604800,"
        "  \"AllowPostInstallReboots\": true"
        "},"
        "\"uc\": {"
        "  \"loglevel\": 7"
        "}"
        "}",IPMLogger::LOG_WARNING,"WARNING"},
    ConfigData{"{"
        "\"id\": {"
        "  \"enable_verbose_logs\": true"
        "},"
        "\"pm\": {"
        "  \"loglevel\": 3,"
        "  \"CheckinInterval\": 300000,"
        "  \"MaxStartupDelay\": 2000,"
        "  \"maxFileCacheAge_s\": 604800,"
        "  \"AllowPostInstallReboots\": true"
        "},"
        "\"uc\": {"
        "  \"loglevel\": 7"
        "}"
        "}",IPMLogger::LOG_ERROR,"ERROR"},
    ConfigData{"{"
        "\"id\": {"
        "  \"enable_verbose_logs\": true"
        "},"
        "\"pm\": {"
        "  \"loglevel\": 2,"
        "  \"CheckinInterval\": 300000,"
        "  \"MaxStartupDelay\": 2000,"
        "  \"maxFileCacheAge_s\": 604800,"
        "  \"AllowPostInstallReboots\": true"
        "},"
        "\"uc\": {"
        "  \"loglevel\": 7"
        "}"
        "}",IPMLogger::LOG_CRITICAL,"CRITICAL"},
    ConfigData{"{"
        "\"id\": {"
        "  \"enable_verbose_logs\": true"
        "},"
        "\"pm\": {"
        "  \"loglevel\": 1,"
        "  \"CheckinInterval\": 300000,"
        "  \"MaxStartupDelay\": 2000,"
        "  \"maxFileCacheAge_s\": 604800,"
        "  \"AllowPostInstallReboots\": true"
        "},"
        "\"uc\": {"
        "  \"loglevel\": 7"
        "}"
        "}",IPMLogger::LOG_ALERT,"ALERT"}
};

std::ostream&
operator <<(std::ostream& os, const ConfigData& sData)
{
    return os << "JSON Config: " << sData.data << "; Expected Log Level: " << sData.logLevelString;
}

} //unnamed namespace

class TestConfiguration : public ::testing::TestWithParam<ConfigData>
{
public:
    void SetUp() override
    {
        filePath_ = std::filesystem::temp_directory_path() / kLogFile;
        testData_ = GetParam();
        PmLogger::initLogger();

        ASSERT_TRUE(createConfigFile());
        pConfig_ = std::make_unique<ConfigShared::Config>(filePath_.native(), &PmLogger::getLogger().getConfigLogger());
//        pConfig_->subscribeForConfigChanges()();
    }
    
    void TearDown() override
    {
        clearConfigFile();
        PmLogger::releaseLogger();
    }
    
protected:
    bool createConfigFile()
    {
        std::fstream out_file(filePath_.native(), std::ios::out);
        if (!out_file.is_open())
            return false;
        out_file << testData_.data;
        out_file.close();
        return true;
    }
    
    bool clearConfigFile()
    {
        if (std::filesystem::exists(filePath_))
        {
            std::error_code errCode;
            return std::filesystem::remove(filePath_, errCode);
        }
        return false;
    }
    
    std::filesystem::path filePath_;
    std::unique_ptr<ConfigShared::Config> pConfig_;
    ConfigData testData_;
};

INSTANTIATE_TEST_SUITE_P(Config,
    TestConfiguration, ::testing::ValuesIn(kConfigValues));

TEST_P(TestConfiguration, getLogLevel)
{
    ASSERT_EQ(testData_.expectedLogLevel, pConfig_->getLogLevel());
}

