/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#include "PMLoader.hpp"
#include "Config.hpp"
#include "Logger/CMLogger.hpp"
#include <memory>

namespace CloudManagement
{

PMLoader::PMLoader() : pm_module_context_{ 0 }
{
    pm_module_context_.nVersion = PM_MODULE_INTERFACE_VERSION;
    CreatePMModuleInstance( &pm_module_context_ );
    pm_module_context_.fpStart(ConfigShared::Config::cmidExePath.c_str(),
                               ConfigShared::Config::cmConfigPath.c_str(),
                               ConfigShared::Config::cmConfigPath.c_str() );
}

PMLoader::~PMLoader()
{
    pm_module_context_.fpStop();
    ReleasePMModuleInstance( &pm_module_context_ );
}

void PMLoader::reloadConfig()
{
    pm_module_context_.fpConfigUpdated();
}

} // namespace ComponentLoader
