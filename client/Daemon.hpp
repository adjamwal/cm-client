/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#pragma once

#ifdef __APPLE__
#include "common/macOS/FileWatcher.hpp"
#include "common/macOS/ProxyWatcher.hpp"
#endif

#ifdef __linux__
#include "common/linux/ProxyWatcher.hpp"
#endif

#include "Config.hpp"

#include <atomic>
#include <thread>
#include <memory>

namespace util
{
class ThreadTimer;
}

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
    std::unique_ptr<ProxyWatcher> proxyWatcher_;

#ifdef __APPLE__
    std::unique_ptr<FileWatcher> fileWatcher_;
#endif

    std::atomic<bool> isRunning_;
    std::thread task_;

    void mainTask();
    void configCallback();
    void applyLoggerSettings();
#ifdef __APPLE__
    void applyCrashpadSettings();
#endif

};

} // namespace CloudManagementClient
