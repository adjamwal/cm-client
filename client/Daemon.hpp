/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#pragma once

#include "common/macOS/FileWatcher.hpp"
#include <atomic>
#include <thread>
#include "Config.hpp"


namespace CloudManagement
{

class CMIDLoader;
class PMLoader;

class Daemon {
public:
    Daemon();
    ~Daemon();

    void start();
    void stop();

private:
    std::unique_ptr<ConfigShared::Config> config_;
    std::unique_ptr<CMIDLoader> cmidLoader_;
    std::unique_ptr<PMLoader> pmLoader_;
    
    std::unique_ptr<FileWatcher> fileWatcher_;
    std::atomic<bool> isRunning_;
    std::thread task_;

    void mainTask();
};

} // namespace CloudManagementClient
