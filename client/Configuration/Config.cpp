/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "Config.hpp"

#include <json/json.h>

#include <filesystem>
#include <iostream>
#include <fstream>

#define LOG_LEVEL_MIN 1
#define LOG_LEVEL_MAX 7

namespace CloudManagement
{

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

Config::Config() 
{
    // Create log path directory (assume data and config paths exists?)
    if (!std::filesystem::is_directory(Config::cmLogPath)) {
        std::filesystem::create_directories(Config::cmLogPath);
    }
#if defined(DEBUG) && defined(CM_CONFIG_PATH) && defined(CM_SHARED_LOG_PATH)
    if (!std::filesystem::is_directory(Config::cmidLogPath)) {
        std::filesystem::create_directories(Config::cmidLogPath);
    }
#endif /* !DEBUG */

    reload();

}

bool Config::reload()
{
    const std::filesystem::path filePath = std::filesystem::path(cmConfigPath) / std::filesystem::path(configFileName);
    return readCmConfig(filePath);
}

bool Config::readCmConfig(const std::filesystem::path &filePath)
{
    if (!std::filesystem::exists(filePath)) {
        CM_LOG_INFO("Config file does not exist.");
        return false;
    }

    Json::CharReaderBuilder builder;
    builder["collectComments"] = true;
    std::ifstream file (filePath.string(), std::ifstream::in);
    Json::Value root;
    JSONCPP_STRING errors;

    if (!parseFromStream(builder, file, &root, &errors)) {
        CM_LOG_INFO("Unable to parse : %s", errors.c_str());
        return false;
    }

    if (!root.isMember(ucKey)) {
        CM_LOG_INFO("missing uc element.");
        return false;
    }

    if (!root[ucKey].isMember(logLevelKey)) {
        CM_LOG_INFO("missing loglevel element.");
        return false;
    }

    if (!root[ucKey][logLevelKey].isInt()) {
        CM_LOG_INFO("loglevel invalid.");
        return false;
    }

    int logLvl_ = root[ucKey][logLevelKey].asInt();

    if ( logLvl_ < LOG_LEVEL_MIN || logLvl_ > LOG_LEVEL_MAX ) {
        CM_LOG_INFO("loglevel value does not fall in the valid range.");
        return false;
    }

    std::lock_guard<std::mutex> lock( mutex_ );
    logLevel_ = static_cast<CM_LOG_LVL_T>(logLvl_);
    return true;

}

CM_LOG_LVL_T Config::getLogLevel() const
{
    std::lock_guard<std::mutex> lock( mutex_ );
    return logLevel_;
}

void Config::onConfigChanged()
{
    if (reload())
    {
        CMLogger::getInstance().setLogLevel(this->getLogLevel());
        CM_LOG_DEBUG("Config succesfully updated");
    }
}

std::function<void()> Config::subscribeForConfigChanges()
{
    return [=]() {
            onConfigChanged();
            };
}


} // namespace CloudManagement