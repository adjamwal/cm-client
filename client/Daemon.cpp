/**
 * @file
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "Daemon.hpp"

#include "ComponentLoader/CMIDLoader.hpp"
#include "ComponentLoader/PMLoader.hpp"
#include "Logger/CMLogger.hpp"
#include "ConfigWatchdog.hpp"
#ifdef __APPLE__
#include "crashpad/CrashpadTuner.h"
#endif
#include "ThreadTimer.hpp"
#include "util/PathUtil.hpp"
#include <sys/stat.h>
#include <chrono>
#include <iostream>

namespace CloudManagement
{
#ifdef __APPLE__
const std::string fileWatcherName {"CloudManagement_FileWatcher"};
#endif

void Daemon::configCallback()
{
    applyLoggerSettings();
#ifdef __APPLE__
    applyCrashpadSettings();
#endif
}
    
void Daemon::applyLoggerSettings() {
    CMLogger::getInstance().SetLogLevel(static_cast<CM_LOG_LVL_T>(config_->getLogLevel()));
}

#ifdef __APPLE__
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
#endif

//! @todo creation of PM, should also load the process
Daemon::Daemon() :
    config_ { std::make_unique<ConfigShared::Config>(&CMLogger::getInstance().getConfigLogger()) },
    cmidLoader_ { std::make_unique<CMIDLoader>() },
    pmLoader_ { std::make_unique<PMLoader>() }
#ifdef __APPLE__
    , fileWatcher_ { std::make_unique<FileWatcher>(fileWatcherName) }
    , proxyWatcher_ { std::make_unique<ProxyWatcher>() }
#endif
{
    applyLoggerSettings();
#ifdef __APPLE__
    applyCrashpadSettings();
#endif
    
    ConfigShared::ConfigWatchdog::getConfigWatchdog().addSubscriber(config_->subscribeForConfigChanges());
    ConfigShared::ConfigWatchdog::getConfigWatchdog().addSubscriber([&](){ this->configCallback(); });
}

void Daemon::start()
{
    using namespace std::chrono_literals;
    std::error_code ecode;
    CM_LOG_DEBUG("Starting cloud management version %s",util::getApplicationVersion().c_str());
    CM_LOG_INFO( "CM: Using config path: %s, exists: %s, code: %d, msg: %s", ConfigShared::Config::cmConfigPath.c_str(),
                 std::filesystem::exists(ConfigShared::Config::cmConfigPath) ? "Yes" : "No", ecode.value(), ecode.message().c_str() );

#ifdef __APPLE__
    fileWatcher_->add(config_->getPath(), []() {ConfigShared::ConfigWatchdog::getConfigWatchdog().detectedConfigChanges();});
#endif
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
