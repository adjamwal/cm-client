/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#pragma once

#include <atomic>
#include <thread>


namespace CloudManagement
{

class CMIDLoader;
class Config;

class Daemon {
public:
    Daemon();
    ~Daemon();

    void start();
    void stop();

private:
    std::unique_ptr<CMIDLoader> cmidLoader_;
    std::unique_ptr<Config> config_;

    std::atomic<bool> isRunning_;
    std::thread task_;

    void mainTask();
};

} // namespace CloudManagementClient
