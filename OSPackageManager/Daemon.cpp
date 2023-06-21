/**
 * @file
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "Daemon.hpp"

#include <sys/stat.h>

#include <chrono>
#include <iostream>
#include "PmPlatformDependencies.hpp"
#include "agent/PackageManagerAgent.hpp"
#include "PmLogger.hpp"

namespace PackageManager
{

Daemon::Daemon()
{
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

void Daemon::mainTask()
{
    umask(0077);
    
    PmPlatformDependencies deps;
    PmLogger logger;
        Agent::PackageManagerAgent agent("/Users/alex/cisco/fork/cm-client/client/config/bs.json", "/Users/alex/cisco/fork/cm-client/client/config/cm_config.json",
                                         deps, logger);
    agent.start();
    GetPMLogger()->Log( IPMLogger::LOG_INFO, "Test output" );

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
}

} // namespace PackageManager
