/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#pragma once

#include <atomic>
#include <thread>

namespace ComponentLoader {
    class CMIDLoader;
}

namespace CloudManagementConfiguration {
    class Config;
}

namespace CloudManagementClient
{

class Daemon {
public:
    Daemon();
    ~Daemon();

    void start();
    void stop();

private:
    std::unique_ptr<ComponentLoader::CMIDLoader> cmidLoader_;
    std::unique_ptr<CloudManagementConfiguration::Config> config_;

    std::atomic<bool> isRunning_;
    std::thread task_;

    void mainTask();

    void init();
};

} // namespace CloudManagementClient
