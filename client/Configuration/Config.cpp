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
    // Create directories (i.e. log paths, data directory, etc...)
    //! @todo make them

    // Do something...
    is_loaded_ = true;
}

} // namespace ComponentLoader
