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

namespace CloudManagementConfiguration
{

using namespace std;

class Config {
public:
#if defined(DEBUG) && defined(CMID_DAEMON_PATH) && defined(CM_CONFIG_PATH) && defined(CM_SHARED_LOG_PATH)
    inline static const string CMID_EXEC_PATH  = CMID_DAEMON_PATH;
    inline static const string CM_CFG_PATH     = CM_CONFIG_PATH;
    inline static const string CM_LOG_PATH     = CM_SHARED_LOG_PATH;
#else
    inline static const string CMID_EXEC_PATH  = "/opt/cisco/secureclient/cloudmanagement/bin";
    inline static const string CM_CFG_PATH     = "/opt/cisco/secureclient/cloudmanagement/etc";
#   ifdef __APPLE__
        inline static const string CM_LOG_PATH = "/Library/Logs/Cisco/Cisco Secure Client/Cloud Management/";
#   else
        inline static const string CM_LOG_PATH = "/var/logs/cisco/secureclient/cloudmanagement/";
#   endif
#endif /* DEBUG */

    Config();
    ~Config() = default;
    Config(const Config &other) = delete;
    Config &operator=(const Config &other) = delete;
    Config(Config &&other) = delete;
    Config &operator=(Config &&other) = delete;

    void load();
    uint32_t getLogLevel();

private:

    std::atomic<bool> is_loaded_;
    uint32_t m_logLevel = 7; //default loglevel
    std::mutex m_mutex;
};

} // namespace ComponentLoader
