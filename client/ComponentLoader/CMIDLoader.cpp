/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "CMIDLoader.hpp"

#include "Configuration/Config.hpp"

#include <memory>

namespace ComponentLoader
{

CMIDLoader::CMIDLoader()
    : is_running_ { false },
      cmid_module_context_{ 0 }
{
    cmid_module_context_.nVersion = PM_MODULE_INTERFACE_VERSION;
    CreateModuleInstance(&cmid_module_context_);
}

CMIDLoader::~CMIDLoader()
{
    if (is_running_) {
        stop();
    }
    ReleaseModuleInstance(&cmid_module_context_);
}

void CMIDLoader::start()
{
    using namespace CloudManagementConfiguration;

    // Does this block?
    cmid_module_context_.fpStart(Config::CMID_EXEC_PATH.c_str(),
                                 Config::CM_CFG_PATH.c_str(),
                                 Config::CM_CFG_PATH.c_str());
    this->is_running_ = true;
}

void CMIDLoader::stop()
{
    cmid_module_context_.fpStop();
    this->is_running_ = false;
}

void CMIDLoader::reloadConfig()
{
    cmid_module_context_.fpConfigUpdated();
}

} // namespace ComponentLoader
