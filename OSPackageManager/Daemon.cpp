/**
 * @file
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "Daemon.hpp"

#include "PmPlatformDependencies.hpp"
#include "agent/PackageManagerAgent.hpp"
#include "configuration/Config.hpp"
#include "PmLogger.hpp"

#include <sys/stat.h>
#include <chrono>
#include <iostream>
#include <filesystem>
#include <string_view>

namespace
{
    constexpr std::string_view kLogFileName = "cmpackagemanager.log";
#ifndef CM_SHARED_LOG_PATH
    constexpr std::string_view kLogDir = "/Library/Logs/Cisco/SecureClient/CloudManagement";
#endif
    constexpr size_t kMaxSize = 1048576 * 15;
    constexpr size_t kMaxFiles = 5;
}

namespace PackageManager
{

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
#endif
#endif
}

void Daemon::start()
{
    isRunning_ = true;
    config_ = std::make_unique<Config>(configFile_);
    PmLogger::getLogger().SetLogLevel(config_->getLogLevel());
    PmLogger::getLogger().initFileLogging(loggerDir_, static_cast<std::string>(kLogFileName),
        kMaxSize, kMaxFiles);
        
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
