/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#pragma once

#include <atomic>
#include <thread>

namespace CloudManagementClient
{

class Daemon {
public:
    ~Daemon();

    void start();
    void stop();

private:
    std::atomic<bool> isRunning_;
    std::thread task_;

    void mainTask();

    void init();
};

} // namespace CloudManagementClient
