#pragma once

#include "config_shared.hpp"
#include <string>
#include <filesystem>
#include <fstream>
#include <shared_mutex>
#include "IConfigLogger.hpp"


namespace ConfigShared
{

#if defined (DEBUG)
#define DEFAULT_LOG_LEVEL 7
#else
#define DEFAULT_LOG_LEVEL 5
#endif

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
    
    Config(const std::string& key, IConfigLogger* logger);
    Config(const std::string& configPath, const std::string& key, IConfigLogger* logger);
    ~Config() = default;
    Config(const Config &other) = delete;
    Config &operator=(const Config &other) = delete;
    Config(Config &&other) = delete;
    Config &operator=(Config &&other) = delete;
    
    std::string getPath() const;
    int getLogLevel() const;

    void onConfigChanged();
    std::function<void()> subscribeForConfigChanges();
    
    IConfigLogger* getConfigLogger();
    void setConfigLogger(IConfigLogger* loger);

private:
    bool reload(); // separate function to allow re-load.
    bool readConfig();

    const std::string logLevelKey = "loglevel";
    const std::string configFileName = "cm_config.json";
    
    std::string key_; //= "uc";  = "pm";
    std::filesystem::path configPath_;

    mutable std::shared_mutex mutex_;
    int logLevel_ = DEFAULT_LOG_LEVEL;
    IConfigLogger* configLogger_{nullptr};

};
    
} // namespace bitsandpieces
