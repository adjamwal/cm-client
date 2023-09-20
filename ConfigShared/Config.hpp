#pragma once

#include <string>
#include <filesystem>
#include <fstream>
#include <shared_mutex>
#include <json/json.h>

#include "config_shared.hpp"
#include "IConfigLogger.hpp"

namespace ConfigShared
{

    namespace log {
        const int kDefaultLevel =
#if defined(DEBUG)
        7
#else
        5
#endif
        ;
    }

    namespace crashpad {
        constexpr uint32_t kPruneDays{14};
        constexpr size_t kDatabaseDefaultPruneSizeKB{50000};
        constexpr std::string_view kCrashpadUrl{"https://crash.qa1.immunet.com/crash"};
    }

    
struct CrashpadConfig {
    std::optional<int> pruneAge;
    std::optional<int> pruneDbSize;
    std::optional<std::string> uploadUrl;
};
    
class CONFIGSHARED_MODULE_API Config
{
public:
    static const std::string cmidExePath;
    static const std::string cmConfigPath;
    static const std::string cmLogPath;
#if defined(DEBUG) && defined(CMID_DAEMON_PATH) && defined(CM_CONFIG_PATH) && defined(CM_SHARED_LOG_PATH)
    /*
     CMID will try to create a log file here in the call to CreateModuleInstance so we need
     to define this separately and build this directory so it's available prior to loading CMID
     */
    static const std::string cmidLogPath;
#endif
    
    Config(IConfigLogger* logger);
    Config(const std::filesystem::path& configPath, IConfigLogger* logger);
    ~Config() = default;
    Config(const Config &other) = delete;
    Config &operator=(const Config &other) = delete;
    Config(Config &&other) = delete;
    Config &operator=(Config &&other) = delete;
    
    const std::filesystem::path& getPath() const;

    void onConfigChanged();
    std::function<void()> subscribeForConfigChanges();
    
    IConfigLogger* getConfigLogger() const;
    void setConfigLogger(IConfigLogger* loger);
    
    int getLogLevel() const;
    const CrashpadConfig& getCrashpadConfig() const;

private:
    bool reload(); // separate function to allow re-load.
    bool readConfig();
    bool parseLogLevel();
    bool parseCrashPadSettings();

    std::filesystem::path configPath_;
    IConfigLogger* configLogger_{nullptr};

    mutable std::shared_mutex mutex_;
    std::unique_ptr<Json::Value> configJson_;
    
    int logLevel_{log::kDefaultLevel};
    CrashpadConfig crashpadConfig_{};
};
    
} // namespace bitsandpieces
