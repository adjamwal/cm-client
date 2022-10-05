/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#pragma once

#include "cmid/PackageManagerInternalModuleAPI.h"

#include <atomic>
#include <thread>


namespace ComponentLoader
{

class CMIDLoader {
public:
    CMIDLoader();
    ~CMIDLoader();
    CMIDLoader(const CMIDLoader &other) = delete;
    CMIDLoader &operator=(const CMIDLoader &other) = delete;
    CMIDLoader(CMIDLoader &&other) = delete;
    CMIDLoader &operator=(CMIDLoader &&other) = delete;

    void start();
    void stop();
    void reloadConfig();

private:
    std::atomic<bool> is_running_;
    PM_MODULE_CTX_T cmid_module_context_;
};

} // namespace ComponentLoader
