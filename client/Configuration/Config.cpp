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

namespace CloudManagementConfiguration
{

Config::Config() {
}

void Config::load()
{

#ifdef DEBUG
    logLevel_ = DEFAULT_LOG_LEVEL_DEBUG;
#else
    logLevel_ = DEFAULT_LOG_LEVEL_RELEASE;
#endif

    const std::string m_filePath = CM_CFG_PATH + PATH_DELIMITER + CONFIG_FILE;


    std::lock_guard<std::mutex> lock( mutex_ );
    is_loaded_ = false;

    // Create log path directory (assume data and config paths exists?)
    if (!filesystem::is_directory(Config::CM_LOG_PATH)) {
        filesystem::create_directories(Config::CM_LOG_PATH);
    }

    std::filesystem::path cfgPath(m_filePath);

    try
    {
        if(!filesystem::exists(cfgPath))
        {
            std::cout << m_filePath << std::endl;
            throw( std::runtime_error( "Config file doesn't exist." ) );
        }
        
        Json::Value root = Config::readCmConfig(m_filePath);
        logLevel_ = root[uc_element][loglevel_element].asInt();
    }
    catch(exception ex)
    {
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