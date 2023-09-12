/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#pragma once

#include "PackageManager/IPmLogger.h"
#include "common/macOS/FileWatcher.hpp"
#include "Config.hpp"


#include <atomic>
#include <thread>
#include <string>
#include <memory>



namespace PackageManager
{

class Daemon {
public:
    Daemon();
    ~Daemon();

    void start();
    void stop();
    
    void setBooststrapPath(const std::string& strPath);
    void setConfigPath(const std::string& strPath);
    void setLoggerDir(const std::string& strLoggerDir);

private:
    std::unique_ptr<ConfigShared::Config> config_;
    std::atomic<bool> isRunning_;
    std::thread task_;
    std::string bootstrap_;
    std::string configFile_;
    std::string loggerDir_;
    std::unique_ptr<FileWatcher> fileWatcher_;

    void mainTask();
};

} // namespace PackageManager
