/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#pragma once

#include <atomic>
#include <thread>
#include <string>

namespace CloudManagementConfiguration
{

using namespace std;

class Config {
public:
#if defined(DEBUG) && defined(CMID_DAEMON_PATH) && defined(CM_CONFIG_PATH) && defined(CM_SHARED_LOG_PATH)
    inline static const string CMID_EXEC_PATH  = CMID_DAEMON_PATH;
    inline static const string CM_CFG_PATH     = CM_CONFIG_PATH;
    inline static const string CM_LOG_PATH     = CM_SHARED_LOG_PATH;
    /*
     CMID will try to create a log file here in the call to CreateModuleInstance so we need
     to define this separately and build this directory so it's available prior to loading CMID
     */
    inline static const string CMID_LOG_PATH   = "/Library/Logs/Cisco/SecureClient/CloudManagement/";
#else
    inline static const string CMID_EXEC_PATH  = "/opt/cisco/secureclient/cloudmanagement/bin";
    inline static const string CM_CFG_PATH     = "/opt/cisco/secureclient/cloudmanagement/etc";
#   ifdef __APPLE__
        inline static const string CM_LOG_PATH = "/Library/Logs/Cisco/SecureClient/CloudManagement/";
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

private:

    std::atomic<bool> is_loaded_;
};

} // namespace ComponentLoader
