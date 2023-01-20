/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#include "PMLoader.hpp"
#include "Configuration/Config.hpp"
#include <memory>

namespace CloudManagement
{

PMLoader::PMLoader() : pm_module_context_{ 0 }
{
    pm_module_context_.nVersion = PM_MODULE_INTERFACE_VERSION;
    CreateModuleInstance( &pm_module_context_ );

    pm_module_context_.fpStart( Config::cmidExePath.c_str(),
                                Config::cmConfigPath.c_str(),
                                Config::cmConfigPath.c_str() );
}

PMLoader::~PMLoader()
{
    pm_module_context_.fpStop();
    ReleaseModuleInstance( &pm_module_context_ );
}

void PMLoader::reloadConfig()
{
    pm_module_context_.fpConfigUpdated();
}

} // namespace ComponentLoader
