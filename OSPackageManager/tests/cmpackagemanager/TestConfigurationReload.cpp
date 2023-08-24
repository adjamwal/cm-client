#include "Config.hpp"
#include "PmLogger.hpp"
#include "ConfigWatchdog.hpp"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <memory>


using namespace PackageManager;

namespace
{
    constexpr std::string_view kLogFile = "CDD95502_343A_4A54_B52E_7C9F0F4E5BA2_log_file.log";
    constexpr std::string_view kConfigFile = "027F3A4D_256A_4D7C_A593_C705289D03E0_config_file.json";

struct ConfigData
{
    std::string data;
    IPMLogger::Severity expectedLogLevel;
    std::string logLevelString;
};

ConfigData testData1_ = ConfigData{"{"
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
        "}",IPMLogger::LOG_DEBUG,"DEBUG"};

    ConfigData testData2_ = ConfigData{"{"
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
        "}",IPMLogger::LOG_ERROR,"ERROR"};

std::ostream&
operator <<(std::ostream& os, const ConfigData& sData)
{
    return os << "JSON Config: " << sData.data << "; Expected Log Level: " << sData.logLevelString;
}

} //unnamed namespace

class TestConfigurationReload : public ::testing::Test
{
public:
    TestConfigurationReload()
    {
        configFilePath_ = std::filesystem::temp_directory_path() / kConfigFile;
        loggerFilePath_ = std::filesystem::temp_directory_path() / kLogFile;
    }
    
    void SetUp() override
    {
        ASSERT_TRUE(createConfigFile(testData1_));
        pConfig_ = std::make_unique<Config>(configFilePath_.native());
        bitsandpieces::ConfigWatchdog::getConfigWatchdog().addSubscriber(pConfig_->subscribeForConfigChanges());
        PmLogger::initLogger();
        PmLogger::getLogger().SetLogLevel(pConfig_->getLogLevel());
        PmLogger::getLogger().initFileLogging(std::filesystem::temp_directory_path(), static_cast<std::string>(kLogFile), 1048576 * 15, 5);
    }
    
    ~TestConfigurationReload()
    {
        PmLogger::releaseLogger();
        clearConfigFile();
        clearLogFile();
    }
    
protected:
    bool createConfigFile(ConfigData testData)
    {
        std::fstream out_file(configFilePath_.native(), std::ios::out);
        if (!out_file.is_open())
            return false;
        out_file << testData.data;
        out_file.close();
        return true;
    }

    bool reCreateConfigFile(ConfigData testData) {
        if(clearConfigFile())
            return createConfigFile(testData2_);
        return false;
    }
    
    bool clearConfigFile()
    {
        if (std::filesystem::exists(configFilePath_))
        {
            std::error_code errCode;
            return std::filesystem::remove(configFilePath_, errCode);
        }
        return false;
    }
    
    bool clearLogFile()
    {
        if (std::filesystem::exists(loggerFilePath_))
        {
            std::error_code errCode;
            return std::filesystem::remove(configFilePath_, errCode);
        }
        return false;
    }

     void checkLogRecordsExists(const std::vector<std::string>& records)
    {
        std::ifstream inputFile;
        inputFile.open(loggerFilePath_.native());
        ASSERT_TRUE(inputFile.is_open());
        

        std::string line;
        size_t i = 0;
        while (std::getline(inputFile, line))
        {
            ASSERT_TRUE(i < records.size());
            EXPECT_TRUE(line.find(records[i]) != std::string::npos);
            ++i;
        }
        EXPECT_TRUE(i == records.size());

        inputFile.close();
    }

    
    std::filesystem::path configFilePath_;
    std::filesystem::path loggerFilePath_;
    std::unique_ptr<Config> pConfig_;
};

TEST_F(TestConfigurationReload, LogLevelChangedAfterReload)
{
    std::vector<std::string> logRecord = {
        "Set new log level: Debug",
        "Set new log level: Error"
    };
    
    ASSERT_EQ(testData1_.expectedLogLevel, pConfig_->getLogLevel());
    
    ASSERT_TRUE(reCreateConfigFile(testData2_));

    bitsandpieces::ConfigWatchdog::getConfigWatchdog().detectedConfigChanges();
    
    logRecord = {
        "Set new log level: Error"
    };

    ASSERT_EQ(testData2_.expectedLogLevel, pConfig_->getLogLevel());
    
    checkLogRecordsExists(logRecord);
}

