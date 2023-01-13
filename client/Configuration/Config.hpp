/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#pragma once

#include <atomic>
#include <thread>
#include <string>
#include <mutex>
#include <json/json.h>


//TODO : replace these with enum from logger
#if defined (DEBUG)
#define DEFAULT_LOG_LEVEL 7
#else
#define DEFAULT_LOG_LEVEL 4
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

    void load();
    uint32_t getLogLevel();

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

private:

    static constexpr const char uc_element[] = "uc";
    static constexpr const char loglevel_element[] = "loglevel";

    std::atomic<bool> is_loaded_;
    uint32_t logLevel_ = DEFAULT_LOG_LEVEL; //default loglevel
    std::mutex mutex_;
    Json::Value readCmConfig(const std::string);
};

} // namespace CloudManagement