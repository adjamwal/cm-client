/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */


//TODO : Add logs using logger 

#include "Config.hpp"

#include <filesystem>
#include <json/json.h>
#include <iostream>
#include <fstream>

#define PATH_DELIMITER "/"
#define CONFIG_FILE "cm_config.json"

namespace CloudManagement
{

#if defined(DEBUG) && defined(CMID_DAEMON_PATH) && defined(CM_CONFIG_PATH) && defined(CM_SHARED_LOG_PATH)
    const std::string Config::cmidExePath  = CMID_DAEMON_PATH;
    const std::string Config::cmConfigPath = CM_CONFIG_PATH;
    const std::string Config::cmLogPath    = CM_SHARED_LOG_PATH;
    /*
     CMID will try to create a log file here in the call to CreateModuleInstance so we need
     to define this separately and build this directory so it's available prior to loading CMID
     */
    const std::string Config::cmidLogPath = "/Library/Logs/Cisco/SecureClient/CloudManagement/";
#else
    const std::string Config::cmidExePath   = "/opt/cisco/secureclient/cloudmanagement/bin";
    const std::string Config::cmConfigPath  = "/opt/cisco/secureclient/cloudmanagement/etc";
#   ifdef __APPLE__
        const std::string Config::cmLogPath = "/Library/Logs/Cisco/SecureClient/CloudManagement/";
#   else
        const std::string Config::cmLogPath = "/var/logs/cisco/secureclient/cloudmanagement/";
#   endif
#endif /* DEBUG */

Config::Config() {
}

void Config::load()
{
    const std::string filePath = CM_CFG_PATH + PATH_DELIMITER + CONFIG_FILE;

    std::lock_guard<std::mutex> lock( mutex_ );
    is_loaded_ = false;

    // Create log path directory (assume data and config paths exists?)
    if (!std::filesystem::is_directory(Config::cmLogPath)) {
        std::filesystem::create_directories(Config::cmLogPath);
    }
#if defined(DEBUG) && defined(CM_CONFIG_PATH) && defined(CM_SHARED_LOG_PATH)
    if (!std::filesystem::is_directory(Config::cmidLogPath)) {
        std::filesystem::create_directories(Config::cmidLogPath);
    }
#endif /* !DEBUG */

    std::filesystem::path cfgPath(filePath);

    try
    {
        if(!filesystem::exists(cfgPath))
        {
            throw( std::runtime_error( "Config file doesn't exist." ) );
        }
        
        Json::Value root = Config::readCmConfig(filePath);
        logLevel_ = root[uc_element][loglevel_element].asInt();
    }
    catch(exception ex)
    {
        logLevel_ = DEFAULT_LOG_LEVEL;
        //TODO : log exception.
    }

    is_loaded_ = true;
}

Json::Value Config::readCmConfig(const std::string filename)
{
    Json::CharReaderBuilder builder;
    builder["collectComments"] = true;
    std::ifstream file (filename, std::ifstream::in);
    Json::Value root;
    JSONCPP_STRING errors;

    if(!parseFromStream(builder, file, &root, &errors))
    {
        throw( std::runtime_error( errors ) );
    }

    if(!root.isMember(uc_element))
    {
        throw( std::runtime_error( "missing uc_service element" ) );
    }

    if(!root[uc_element][loglevel_element].isInt())
    {
        throw( std::runtime_error( "loglevel missing or invalid." ) );
    }

    return root;

}

uint32_t Config::getLogLevel()
{
    std::lock_guard<std::mutex> lock( mutex_ );
    return logLevel_;
}

} // namespace CloudManagementConfiguration