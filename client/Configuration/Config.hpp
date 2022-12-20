/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#pragma once

#include <atomic>
#include <thread>
#include <string>

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

    std::atomic<bool> is_loaded_;
};

} // namespace CloudManagement
