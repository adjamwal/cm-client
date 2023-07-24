/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#pragma once

#include "Logger/CMLogger.hpp"

#include <json/json.h>

#include <atomic>
#include <thread>
#include <string>
#include <mutex>

#if defined (DEBUG)
#define DEFAULT_LOG_LEVEL CM_LOG_LVL_T::CM_LOG_DEBUG
#else
#define DEFAULT_LOG_LEVEL CM_LOG_LVL_T::CM_LOG_NOTICE
#endif

namespace CloudManagement
{

class Config {
public:
    Config();
    ~Config() = default;
    Config(const Config &other) = delete;
    Config &operator=(const Config &other) = delete;
    Config(Config &&other) = delete;
    Config &operator=(Config &&other) = delete;

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

    bool reload(); // separate function to allow re-load. 
    CM_LOG_LVL_T getLogLevel() const;
    

private:
    
    bool readCmConfig(const std::filesystem::path&);

    const std::string ucKey = "uc";
    const std::string logLevelKey = "loglevel";
    const std::string configFileName = "cm_config.json";

    mutable std::mutex mutex_;
    CM_LOG_LVL_T logLevel_ = DEFAULT_LOG_LEVEL;
};

} // namespace CloudManagement
