/**
 * @file
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "Daemon.hpp"

#include "ComponentLoader/CMIDLoader.hpp"
#include "ComponentLoader/PMLoader.hpp"
#include "Logger/CMLogger.hpp"
#include "ConfigWatchdog.hpp"

#include <sys/stat.h>
#include <chrono>
#include <iostream>

namespace CloudManagement
{
const std::string fileWatcherName {"CloudManagement_FileWatcher"};

void configCallback()
{
//TODO: place for config update integration
}

//! @todo creation of PM, should also load the process
Daemon::Daemon()
#ifdef CM_KEY
    : config_ { std::make_unique<ConfigShared::Config>(CM_KEY) },
#else 
    : config_ { std::make_unique<ConfigShared::Config>("uc", &CMLogger::getInstance().getConfigLogger()) },
#endif
      cmidLoader_ { std::make_unique<CMIDLoader>() },
      pmLoader_ { std::make_unique<PMLoader>() },
      fileWatcher_{std::make_unique<FileWatcher>(fileWatcherName)}
{
    CMLogger::getInstance().SetLogLevel(static_cast<CM_LOG_LVL_T>(config_->getLogLevel()));
    ConfigShared::ConfigWatchdog::getConfigWatchdog().addSubscriber(config_->subscribeForConfigChanges());
}

void Daemon::start()
{
    CM_LOG_DEBUG("Starting cloud management");
 
    fileWatcher_->add(config_->getPath(), []() {ConfigShared::ConfigWatchdog::getConfigWatchdog().detectedConfigChanges();});
    isRunning_ = true;

    task_ = std::thread(&Daemon::mainTask, this);
    task_.join();
}

void Daemon::stop()
{
    isRunning_ = false;
}

Daemon::~Daemon()
{
}

void Daemon::mainTask()
{

    umask(0077);

    //! TODO: Just busy wait??
    //!
    //! Change as needed...
    while(isRunning_) {
        using namespace std;
        using namespace std::chrono_literals;

        cout << "Just chillin here..." << endl;

        //auto start = chrono::high_resolution_clock::now();
        (void) chrono::high_resolution_clock::now();
        this_thread::sleep_for(1000ms);
        //auto end = chrono::high_resolution_clock::now();
        (void) chrono::high_resolution_clock::now();
    }
}

} // namespace CloudManagementClient
