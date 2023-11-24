#include "Config.hpp"
#include <cassert>

namespace ConfigShared
{
    constexpr int minLogLevel = 1;
    constexpr int maxLogLevel = 7;

    const std::string configFileName = "cm_config.json";
    const std::string logLevelKey = "loglevel";

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


Config::Config(IConfigLogger* logger) :
    configPath_(std::filesystem::path(cmConfigPath) / configFileName), configLogger_(logger)
{
    onConfigChanged();
}

Config::Config(const std::filesystem::path& configPath, IConfigLogger* logger) :
    configPath_(configPath), configLogger_(logger)
{
    onConfigChanged();
}

bool Config::readConfig()
{
    std::ifstream file(configPath_.native(), std::ifstream::in);
    if (!file.is_open()) {
        CONFIG_LOG_INFO("config file % is not opened", configPath_.string().c_str());
        return false;
    }

    std::unique_lock<std::shared_mutex> _(mutex_);
    this->configJson_.reset( new Json::Value() );

    JSONCPP_STRING errors;
    Json::CharReaderBuilder builder;
    builder["collectComments"] = true;
    if (!parseFromStream(builder, file, configJson_.get(), &errors)) {
        CONFIG_LOG_INFO("Unable to parse %s: %s", configPath_.string().c_str(), errors.c_str());
        return false;
    }
    return true;
}
    
bool Config::parseLogLevel() {
    assert(configLogger_);
    const auto configKey = configLogger_->getKey();
    
    assert(configJson_);
    if (!configJson_->isMember(configKey)) {
        CONFIG_LOG_INFO("missing '%s' element.", configKey);
        return false;
    }
    
    const auto& jsonKeyValue = (*configJson_)[configKey];
    
    if (!jsonKeyValue.isMember(logLevelKey)) {
        CONFIG_LOG_INFO("missing 'loglevel' element.");
        return false;
    }
    
    if (!jsonKeyValue[logLevelKey].isInt()) {
        CONFIG_LOG_WARNING("'loglevel' has invalid value type.");
        return false;
    }
    
    const int levelValue = jsonKeyValue[logLevelKey].asInt();
    
    if ( levelValue < minLogLevel || levelValue > maxLogLevel ) {
        CONFIG_LOG_INFO("loglevel value does not fall in the valid range.");
        return false;
    }
    
    logLevel_ = levelValue;
    return true;
}
    
bool Config::parseCrashPadSettings() {
    assert(configJson_);
    if (!configJson_->isMember("crashpad")) {
        CONFIG_LOG_WARNING("missing 'crashpad' element.");
        return false;
    }
    
    const auto& crashpadJson = (*configJson_)["crashpad"];
    assert(crashpadJson.isObject());
    if (!crashpadJson.isObject()) {
        CONFIG_LOG_WARNING("'crashpad' has invalid value type");
        return false;
    }
    
    if (crashpadJson.isMember("pruneAge") && crashpadJson["pruneAge"].isInt()) {
        crashpadConfig_.pruneAge = crashpadJson["pruneAge"].asInt();
    } else {
        CONFIG_LOG_WARNING("'pruneAge' is missing or invalid in 'crashpad'.");
        return false;
    }
    
    if (crashpadJson.isMember("pruneDbSize") && crashpadJson["pruneDbSize"].isInt()) {
        crashpadConfig_.pruneDbSize = crashpadJson["pruneDbSize"].asInt();
    } else {
        CONFIG_LOG_WARNING("'pruneDbSize' is missing or invalid in 'crashpad'.");
        return false;
    }
    
    if (crashpadJson.isMember("uploadUrl") && crashpadJson["uploadUrl"].isString()) {
        crashpadConfig_.uploadUrl = crashpadJson["uploadUrl"].asString();
    } else {
        CONFIG_LOG_WARNING("'uploadUrl' is missing or invalid in 'crashpad'.");
        return false;
    }

    return true;
}

const std::filesystem::path& Config::getPath() const
{
    return configPath_;
}

int Config::getLogLevel() const {
    std::shared_lock<std::shared_mutex> _(mutex_);
    return logLevel_;
}
    
const CrashpadConfig& Config::getCrashpadConfig() const {
    std::shared_lock<std::shared_mutex> _(mutex_);
    return crashpadConfig_;
}

bool Config::reload()
{
    if (!std::filesystem::exists(configPath_)) {
        CONFIG_LOG_ERROR("Config path %s does not exist.", configPath_.string().c_str());
        return false;
    }
    
    if (!std::filesystem::is_regular_file(configPath_)) {
        CONFIG_LOG_ERROR("Config path %s does not correspond to a regular file.", configPath_.string().c_str());
        return false;
    }

    if (!readConfig())
        return false;
    
    parseLogLevel();
    parseCrashPadSettings();
    return true;
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


IConfigLogger* Config::getConfigLogger() const
{
    return configLogger_;
}

void Config::setConfigLogger(IConfigLogger* logger)
{
    this->configLogger_ = logger;
}

}


