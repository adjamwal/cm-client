/**
 * @file
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#include "Config.hpp"

#include "PmLogger.hpp"
#include <json/json.h>
#include <fstream>

using namespace PackageManager;

namespace
{
const char* kPmRootKey = "pm";
const char* kLogLevelKey = "loglevel";
}

Config::Config(const std::string& configPath)
{
    reload(configPath);
}

bool Config::reload(const std::string& configPath)
{
    if (!std::filesystem::exists(configPath)) {
        PM_LOG_WARNING("Config file does not exist.");
        return false;
    }
    configPath_ = configPath;
    return parsePmConfig();
}

bool Config::parsePmConfig()
{
    Json::CharReaderBuilder builder;
    builder["collectComments"] = true;
    std::ifstream file (configPath_.native(), std::ifstream::in);
    Json::Value root;
    JSONCPP_STRING errors;

    if (!parseFromStream(builder, file, &root, &errors)) {
        PM_LOG_ERROR("Unable to parse : %s", errors.c_str());
        return false;
    }

    if (!root.isMember(kPmRootKey)) {
        PM_LOG_ERROR("missing %s element.", kPmRootKey);
        return false;
    }
    const Json::Value pmRoot = root[kPmRootKey];

    if (!pmRoot.isMember(kLogLevelKey)) {
        PM_LOG_ERROR("missing %s element.", kLogLevelKey);
        return false;
    }
    const Json::Value logLevelV = pmRoot[kLogLevelKey];

    if (!logLevelV.isInt()) {
        PM_LOG_ERROR("loglevel invalid.");
        return false;
    }
    const int logLevel = logLevelV.asInt();

    if ( logLevel < IPMLogger::LOG_ALERT || logLevel > IPMLogger::LOG_DEBUG ) {
        PM_LOG_ERROR("loglevel value does not fall in the valid range.");
        return false;
    }
    logLevel_ = static_cast<IPMLogger::Severity>(logLevel);
    return true;
}

IPMLogger::Severity Config::getLogLevel() const
{
    return logLevel_;
}

IPMLogger::Severity Config::getDefaultLogLevel()
{
#if defined (DEBUG)
    return IPMLogger::LOG_DEBUG;
#else
    return IPMLogger::LOG_NOTICE;
#endif
}


void Config::onConfigChanged()
{     
    if(reload(configPath_)) 
    {
        PmLogger::getLogger().SetLogLevel(this->getLogLevel());
        PM_LOG_DEBUG("Config succesfully updated");
    }
}

std::function<void()> Config::subscribeForConfigChanges()
{
    return [=]() {
            onConfigChanged();
        };
}