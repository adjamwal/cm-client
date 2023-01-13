/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#pragma once

#include "cmid/PackageManagerInternalModuleAPI.h"

#include <atomic>
#include <thread>


namespace CloudManagement
{

class CMIDLoader {
public:
    CMIDLoader();
    ~CMIDLoader();
    CMIDLoader(const CMIDLoader &other) = delete;
    CMIDLoader &operator=(const CMIDLoader &other) = delete;
    CMIDLoader(CMIDLoader &&other) = delete;
    CMIDLoader &operator=(CMIDLoader &&other) = delete;

    void reloadConfig();

private:
    PM_MODULE_CTX_T cmid_module_context_;
};

} // namespace CloudManagement
