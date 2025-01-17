/**
 * @file
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "Daemon.hpp"

#include "PmPlatformDependencies.hpp"
#include "agent/PackageManagerAgent.hpp"
#include "PmLogger.hpp"
#include "ConfigWatchdog.hpp"
#ifdef __APPLE__
#include "ProxyDiscovery/IProxyLogger.h"
#endif

#include <sys/stat.h>
#include <cassert>
#include <chrono>
#include <iostream>
#include <filesystem>
#include <string_view>

namespace
{
    constexpr std::string_view kLogFileName = "cmpackagemanager.log";
#ifndef CM_SHARED_LOG_PATH
#ifdef __APPLE__
    constexpr std::string_view kLogDir = "/Library/Logs/Cisco/SecureClient/CloudManagement";
#elif __linux__
    constexpr std::string_view kLogDir = "/var/logs/cisco/secureclient/cloudmanagement/";
#endif // APPLE / linux
#endif // CM_ShARED_LOG_PATH
    constexpr size_t kMaxSize = 1048576 * 15;
    constexpr size_t kMaxFiles = 5;
}

namespace PackageManager
{
const std::string fileWatcherName {"PackageManager_FileWatcher"};

Daemon::Daemon()
{
#ifdef CM_CONFIG_PATH
    bootstrap_ = (std::filesystem::path(CM_CONFIG_PATH) /
                  std::filesystem::path("bs.json")).native();
    configFile_ = (std::filesystem::path(CM_CONFIG_PATH) /
                   std::filesystem::path("cm_config.json")).native();
#endif
#ifdef CM_SHARED_LOG_PATH
    setLoggerDir(CM_SHARED_LOG_PATH);
#else
#ifdef __APPLE__
    setLoggerDir(static_cast<std::string>(kLogDir));
#elif __linux__
    setLoggerDir(static_cast<std::string>(kLogDir));
#endif
#endif
#ifdef __APPLE__
    /** @note Implement for Linux */
    fileWatcher_ = std::make_unique<FileWatcher>(fileWatcherName);
#endif /* __APPLE__ */
}

void Daemon::start()
{
    isRunning_ = true;
    config_ = std::make_unique<ConfigShared::Config>(configFile_, &PmLogger::getLogger().getConfigLogger());
#ifdef __APPLE__
    fileWatcher_->add(config_->getPath(), []() {ConfigShared::ConfigWatchdog::getConfigWatchdog().detectedConfigChanges();});
#endif /* __APPLE__ */
    ConfigShared::ConfigWatchdog::getConfigWatchdog().addSubscriber(config_->subscribeForConfigChanges());
    PmLogger::getLogger().SetLogLevel(static_cast<IPMLogger::Severity>(config_->getLogLevel()));
    PmLogger::getLogger().initFileLogging(loggerDir_, static_cast<std::string>(kLogFileName),
        kMaxSize, kMaxFiles);
#ifdef __APPLE__
    proxy::SetProxyLogger(&PmLogger::getLogger().getProxyLogger());
#endif
    config_->setConfigLogger(&PmLogger::getLogger().getConfigLogger());
        
    task_ = std::thread(&Daemon::mainTask, this);
    task_.join();
}

void Daemon::stop()
{
    isRunning_ = false;
}

Daemon::~Daemon()
{
    stop();
}

void Daemon::setBooststrapPath(const std::string& strPath)
{
    std::filesystem::path p = std::filesystem::path(strPath);
    if(std::filesystem::exists(p))
    {
        bootstrap_ = p;
        PM_LOG_INFO( "PM Using bootstrap path: %s", bootstrap_.c_str() );
    }
    else
    {
        PM_LOG_ERROR("Non existing path to the bootsrap: %s", strPath.c_str());
    }
}

void Daemon::setConfigPath(const std::string& strPath)
{
    std::filesystem::path p = std::filesystem::path(strPath);
    if(std::filesystem::exists(p))
    {
        configFile_ = p;
        PM_LOG_INFO( "PM Using config path: %s", configFile_.c_str() );
    }
    else
    {
        PM_LOG_ERROR("Non existing path to the config file: %s", strPath.c_str());
    }
}

void Daemon::setLoggerDir(const std::string& strLoggerDir)
{
    try
    {
        std::filesystem::path p = std::filesystem::path(strLoggerDir);
        if (!std::filesystem::exists(p))
        {
            std::filesystem::create_directories(p);
            loggerDir_ = p;
        }
        loggerDir_ = p;

        PM_LOG_INFO( "PM Using logged directory path: %s", loggerDir_.c_str() );
    }
    catch(std::exception& ex)
    {
        PM_LOG_ERROR("Filed to create non existing logger directory: %s. Exception occured: %s", strLoggerDir.c_str(), ex.what());
    }
}

void Daemon::mainTask()
{
    umask(0077);
    
    assert(GetPMLogger() != nullptr);
    PmPlatformDependencies deps;
    Agent::PackageManagerAgent agent(bootstrap_, configFile_, deps, PmLogger::getLogger());
    agent.start();

    //! TODO: Just busy wait??
    //!
    //! Change as needed...
    while(isRunning_) {
        using namespace std;
        using namespace std::chrono_literals;

        cout << "PM Just chillin here..." << endl;
        //auto start = chrono::high_resolution_clock::now();
        (void) chrono::high_resolution_clock::now();
        this_thread::sleep_for(1000ms);
        //auto end = chrono::high_resolution_clock::now();
        (void) chrono::high_resolution_clock::now();
    }
    agent.stop();
}

} // namespace PackageManager
