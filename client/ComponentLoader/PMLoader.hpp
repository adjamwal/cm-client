/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#pragma once

#include "OSPackageManager/controlplugin/ModuleControlPlugin.hpp"
#include <atomic>
#include <thread>

namespace CloudManagement
{

class PMLoader {
public:
    PMLoader();
    ~PMLoader();
    PMLoader(const PMLoader &other) = delete;
    PMLoader &operator=(const PMLoader &other) = delete;
    PMLoader(PMLoader &&other) = delete;
    PMLoader &operator=(PMLoader &&other) = delete;

    void reloadConfig();

private:
    PM_MODULE_CTX_T pm_module_context_;
};

} // namespace CloudManagement
