/**
 * @file
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "Daemon.hpp"

#include <sys/stat.h>

#include <chrono>
#include <iostream>

namespace PackageManager
{

Daemon::Daemon()
{
}

void Daemon::init()
{
}

void Daemon::start()
{
    this->isRunning_ = true;

    this->task_ = std::thread(&Daemon::mainTask, this);
    this->task_.join();
}

void Daemon::stop()
{
    this->isRunning_ = false;
}

Daemon::~Daemon()
{
    this->stop();
}

void Daemon::mainTask()
{
    init();

    //! TODO: Just busy wait??
    //!
    //! Change as needed...
    while(this->isRunning_) {
        using namespace std;
        using namespace std::chrono_literals;

        umask(0077);

        cout << "Just chillin here..." << endl;

        //auto start = chrono::high_resolution_clock::now();
        (void) chrono::high_resolution_clock::now();
        this_thread::sleep_for(1000ms);
        //auto end = chrono::high_resolution_clock::now();
        (void) chrono::high_resolution_clock::now();
    }
}

} // namespace PackageManager
