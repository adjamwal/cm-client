/**
 * @file
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "Daemon.hpp"

#include "ComponentLoader/CMIDLoader.hpp"
#include "ComponentLoader/PMLoader.hpp"
#include "Configuration/Config.hpp"
#include "Logger/CMLogger.hpp"
#include "Configuration/ConfigWatchdog.hpp"

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
    : config_ { std::make_unique<Config>() },
      cmidLoader_ { std::make_unique<CMIDLoader>() },
      pmLoader_ { std::make_unique<PMLoader>() },
      fileWatcher_{std::make_unique<FileWatcher>(fileWatcherName)}
{
    CMLogger::getInstance().setLogLevel(config_->getLogLevel());
    ConfigWatchdog::getConfigWatchdog().addSubscriber(config_->subscribeForConfigChanges());
}

void Daemon::start()
{
    CM_LOG_DEBUG("Starting cloud management");
 
    fileWatcher_->add(config_->getPath(), []() {ConfigWatchdog::getConfigWatchdog().detectedСonfigСhanges();});
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
