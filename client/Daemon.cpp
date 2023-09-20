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

#include "crashpad/CrashpadTuner.h"

namespace CloudManagement
{
const std::string fileWatcherName {"CloudManagement_FileWatcher"};

void Daemon::configCallback()
{
    applyLoggerSettings();
    applyCrashpadSettings();
}
    
void Daemon::applyLoggerSettings() {
    CMLogger::getInstance().SetLogLevel(static_cast<CM_LOG_LVL_T>(config_->getLogLevel()));
}
    
void Daemon::applyCrashpadSettings() {
    auto *pCrashpadTuner = CrashpadTuner::getInstance();
    const auto& crashpadConfig = config_->getCrashpadConfig();
    if (crashpadConfig.pruneAge.has_value())
        pCrashpadTuner->setPruneAge( crashpadConfig.pruneAge.value() );
    
    if (crashpadConfig.pruneDbSize.has_value())
        pCrashpadTuner->setPruneDatabaseSize( crashpadConfig.pruneDbSize.value() );
    
    if (crashpadConfig.uploadUrl.has_value())
        pCrashpadTuner->setUploadUrl( crashpadConfig.uploadUrl.value() );
}
    
    
//! @todo creation of PM, should also load the process
Daemon::Daemon()
    : config_ { std::make_unique<ConfigShared::Config>(&CMLogger::getInstance().getConfigLogger()) },
      cmidLoader_ { std::make_unique<CMIDLoader>() },
      pmLoader_ { std::make_unique<PMLoader>() },
      fileWatcher_{std::make_unique<FileWatcher>(fileWatcherName)}
{
    applyLoggerSettings();
    applyCrashpadSettings();
    
    ConfigShared::ConfigWatchdog::getConfigWatchdog().addSubscriber(config_->subscribeForConfigChanges());
    ConfigShared::ConfigWatchdog::getConfigWatchdog().addSubscriber([&](){ this->configCallback(); });
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
