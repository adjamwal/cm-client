/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "Config.hpp"

#include <filesystem>

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
    // Create log path directory (assume data and config paths exists?)
    if (!std::filesystem::is_directory(Config::cmLogPath)) {
        std::filesystem::create_directories(Config::cmLogPath);
    }
#if defined(DEBUG) && defined(CM_CONFIG_PATH) && defined(CM_SHARED_LOG_PATH)
    if (!std::filesystem::is_directory(Config::cmidLogPath)) {
        std::filesystem::create_directories(Config::cmidLogPath);
    }
#endif /* !DEBUG */

    // Do something...
    is_loaded_ = true;
}

} // namespace ComponentLoader
