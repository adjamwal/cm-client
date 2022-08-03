/**
 * @file
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "Daemon.hpp"

#include <sys/stat.h>

#include <chrono>
#include <iostream>

namespace CloudManagementClient
{

void Daemon::start() {
    this->isRunning_ = true;

    this->task_ = std::thread(&Daemon::mainTask, this);

    while (this->isRunning_) {
        using namespace std::chrono_literals;

        //! Wait...

        auto start = std::chrono::high_resolution_clock::now();
        std::this_thread::sleep_for(1000ms);
        auto end = std::chrono::high_resolution_clock::now();
    }
}

void Daemon::stop() {
    this->isRunning_ = false;
    task_.join();
}

Daemon::~Daemon() {
     this->stop();
}        

void Daemon::mainTask()
{
    //! TODO: Make this actually do things...
    //!
    //! Entire contents below will be removed
    while(this->isRunning_) {
        using namespace std::chrono_literals;

        umask(0);

        std::cout << "Do things here!" << std::endl;

        auto start = std::chrono::high_resolution_clock::now();
        std::this_thread::sleep_for(2000ms);
        auto end = std::chrono::high_resolution_clock::now();
    }
}

} // namespace CloudManagementClient
