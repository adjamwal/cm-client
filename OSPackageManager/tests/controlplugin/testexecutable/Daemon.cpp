/**
 * @file
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#include "Daemon.hpp"

#include <sys/stat.h>

#include <chrono>
#include <iostream>

namespace PackageManagerTest
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

    //! TODO: Just busy wait??
    //!
    //! Change as needed...
    while(isRunning_) {
        using namespace std;
        using namespace std::chrono_literals;

        cout << "Test PM Just chillin here..." << endl;

        (void) chrono::high_resolution_clock::now();
        this_thread::sleep_for(1000ms);
        (void) chrono::high_resolution_clock::now();
    }
}

} // namespace PackageManagerTest
