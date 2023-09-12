/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "CMIDLoader.hpp"

#include "Config.hpp"

#include <memory>

namespace CloudManagement
{

CMIDLoader::CMIDLoader()
    : cmid_module_context_{ 0 }
{
    cmid_module_context_.nVersion = PM_MODULE_INTERFACE_VERSION;
    CreateModuleInstance(&cmid_module_context_);

    // Does this block?
    cmid_module_context_.fpStart(ConfigShared::Config::cmidExePath.c_str(),
                                 ConfigShared::Config::cmConfigPath.c_str(),
                                 ConfigShared::Config::cmConfigPath.c_str());
}

CMIDLoader::~CMIDLoader()
{
    cmid_module_context_.fpStop();
    ReleaseModuleInstance(&cmid_module_context_);
}

void CMIDLoader::reloadConfig()
{
    cmid_module_context_.fpConfigUpdated();
}

} // namespace ComponentLoader
