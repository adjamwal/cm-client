/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "Config.hpp"

#include <filesystem>
#include <json/value.h>
#include <json/json.h>
#include <iostream>
#include <fstream>


namespace CloudManagementConfiguration
{

Config::Config() {
}

void Config::load()
{

    std::lock_guard<std::mutex> lock( m_mutex );
    is_loaded_ = false;

    // Create log path directory (assume data and config paths exists?)
    if (!filesystem::is_directory(Config::CM_LOG_PATH)) {
        filesystem::create_directories(Config::CM_LOG_PATH);
    }

    std::filesystem::path cfgPath(CM_CFG_PATH+"/"+"cm_config.json");

    if(!filesystem::exists(cfgPath))
    {
        std::cout << "Config file doesn't exist." << std::endl;
        return;
    }

    Json::CharReaderBuilder builder;
    builder["collectComments"] = true;
    std::ifstream file;
    file.open(CM_CFG_PATH+"/"+"cm_config.json");
    Json::Value root;
    JSONCPP_STRING errors;

    if(!parseFromStream(builder, file, &root, &errors))
    {
        std::cout << errors << std::endl;
        return;
    }

    if(!root.isMember("uc"))
    {
        std::cout << "missing uc_service element" << std::endl;
        return;
    }

    if(!root["uc"]["loglevel"].isInt())
    {
        std::cout << "loglevel missing or invalid." << std::endl;
        return;
    }

    //std::cout << root << std::endl;
    //std::cout << root["loglevel"] << std::endl;
    m_logLevel = root["uc"]["loglevel"].asInt();
    is_loaded_ = true;
}

int Config::getLogLevel()
{
    std::lock_guard<std::mutex> lock( m_mutex );
    return m_logLevel;
}

} // namespace ComponentLoader
