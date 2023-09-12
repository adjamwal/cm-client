#include "Config.hpp"

#include <json/json.h>

namespace ConfigShared
{

#define CM_KEY "uc"
#define PM_KEY "pm"

#define LOG_LEVEL_MIN 1
#define LOG_LEVEL_MAX 7

#if defined(DEBUG) && defined(CMID_DAEMON_PATH) && defined(CM_CONFIG_PATH) && defined(CM_SHARED_LOG_PATH) && defined(CMID_LOG_PATH)
    const std::string Config::cmidExePath  = CMID_DAEMON_PATH;
    const std::string Config::cmConfigPath = CM_CONFIG_PATH;
    const std::string Config::cmLogPath    = CM_SHARED_LOG_PATH;
    /*
     CMID will try to create a log file here in the call to CreateModuleInstance so we need
     to define this separately and build this directory so it's available prior to loading CMID
     */
    const std::string Config::cmidLogPath = CMID_LOG_PATH;
#else
    const std::string Config::cmidExePath   = "/opt/cisco/secureclient/cloudmanagement/bin";
    const std::string Config::cmConfigPath  = "/opt/cisco/secureclient/cloudmanagement/etc";
#   ifdef __APPLE__
        const std::string Config::cmLogPath = "/Library/Logs/Cisco/SecureClient/CloudManagement/";
#   else
        const std::string Config::cmLogPath = "/var/logs/cisco/secureclient/cloudmanagement/";
#   endif
#endif /* DEBUG */


Config::Config(const std::string& key, IConfigLogger* logger) :
    key_(key), configLogger_(logger)
{
}

Config::Config(const std::string& configPath, const std::string& key, IConfigLogger* logger) :
    Config(key, logger)
{
    configPath_ = configPath;
}

bool Config::readConfig()
{
    Json::CharReaderBuilder builder;
    builder["collectComments"] = true;
    std::ifstream file (configPath_.native(), std::ifstream::in);
    Json::Value root;
    JSONCPP_STRING errors;

    if (!parseFromStream(builder, file, &root, &errors)) {
        CONFIG_LOG_INFO("Unable to parse %s: %s", configPath_.string().c_str(), errors.c_str());
        return false;
    }

    if (!root.isMember(key_)) {
        CONFIG_LOG_INFO("missing '%s' element.", key_.c_str());
        return false;
    }

    if (!root[key_].isMember(logLevelKey)) {
        CONFIG_LOG_INFO("missing 'loglevel' element.");
        return false;
    }

    if (!root[key_][logLevelKey].isInt()) {
        CONFIG_LOG_WARNING("'loglevel' has invalid value type.");
        return false;
    }

    const int levelValue = root[key_][logLevelKey].asInt();

    if ( levelValue < LOG_LEVEL_MIN || levelValue > LOG_LEVEL_MAX ) {
        CONFIG_LOG_INFO("loglevel value does not fall in the valid range.");
        return false;
    }

    std::unique_lock<std::shared_mutex> _(mutex_);
    logLevel_ = levelValue;
    return true;

}

std::string Config::getPath() const
{
    return configPath_.string();
}

int Config::getLogLevel() const {
    std::shared_lock<std::shared_mutex> _(mutex_);
    return logLevel_;
}

bool Config::reload()
{
    if (!std::filesystem::exists(configPath_)) {
        CONFIG_LOG_WARNING("Config file does not exist.");
        return false;
    }
    return readConfig();
}

void Config::onConfigChanged()
{     
    if(reload()) 
    {
        assert(configLogger_);
        configLogger_->SetLogLevel(this->getLogLevel());
        CONFIG_LOG_DEBUG("Config succesfully updated");
    }
}

std::function<void()> Config::subscribeForConfigChanges()
{
    return [&]() {
        onConfigChanged();
    };
}


IConfigLogger* Config::getConfigLogger()
{
    return configLogger_;
}

void Config::setConfigLogger(IConfigLogger* logger)
{
    this->configLogger_ = logger;
}

}


