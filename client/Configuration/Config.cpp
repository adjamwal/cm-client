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

    // Do something...
    is_loaded_ = true;
}

} // namespace ComponentLoader
