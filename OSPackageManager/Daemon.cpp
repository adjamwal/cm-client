/**
 * @file
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "Daemon.hpp"

#include "PmPlatformDependencies.hpp"
#include "agent/PackageManagerAgent.hpp"
#include "PmLogger.hpp"

#include <sys/stat.h>
#include <chrono>
#include <iostream>
#include <filesystem>

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
}

void Daemon::start()
{
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
        //LOG_ERROR("Non existing path to the bootsrap: %s", strPath);
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
        //LOG_ERROR("Non existing path to the log file: %s", strPath);
    }
}

void Daemon::mainTask()
{
    umask(0077);
    
    assert(GetPMLogger() != nullptr);
    PmPlatformDependencies deps;
    Agent::PackageManagerAgent agent(bootstrap_, configFile_, deps, PmLogger::GetCurrentLogger());
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
