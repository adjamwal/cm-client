/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#pragma once

#include "PackageManager/IPmLogger.h"

#include <atomic>
#include <thread>
#include <string>
#include <memory>

namespace PackageManager
{
class Config;

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
    std::unique_ptr<Config> config_;
    std::atomic<bool> isRunning_;
    std::thread task_;
    std::string bootstrap_;
    std::string configFile_;
    std::string loggerDir_;

    void mainTask();
};

} // namespace PackageManager
