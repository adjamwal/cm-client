/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "Config.hpp"

#include <filesystem>

namespace CloudManagementConfiguration
{

Config::Config() {
}

void Config::load()
{
    // Create log path directory (assume data and config paths exists?)
    if (!filesystem::is_directory(Config::CM_LOG_PATH)) {
        filesystem::create_directories(Config::CM_LOG_PATH);
    }
#if defined(DEBUG) && defined(CM_CONFIG_PATH) && defined(CM_SHARED_LOG_PATH)
    if (!filesystem::is_directory(Config::CMID_LOG_PATH)) {
        filesystem::create_directories(Config::CMID_LOG_PATH);
    }
#endif /* !DEBUG */

    // Do something...
    is_loaded_ = true;
}

} // namespace ComponentLoader
