/**
 * @file
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "Daemon.hpp"

#include "ComponentLoader/CMIDLoader.hpp"
#include "Configuration/Config.hpp"

#include "cmid/PackageManagerInternalModuleAPI.h"

#include <sys/stat.h>

#include <chrono>
#include <iostream>

using namespace CloudManagementConfiguration;

namespace CloudManagementClient
{

void Daemon::start() {
    this->isRunning_ = true;

    this->task_ = std::thread(&Daemon::mainTask, this);
    this->task_.join();
}

void Daemon::stop() {
    this->isRunning_ = false;
}

Daemon::~Daemon() {
    this->stop();
}

void Daemon::init()
{
}

void Daemon::mainTask()
{
    using namespace cmid;

    init();

    PM_MODULE_CTX_T  modContext = { 0 };
    modContext.nVersion = PM_MODULE_INTERFACE_VERSION;
    CreateModuleInstance(&modContext);

    /** @todo replace 2nd parameter with data directory */
    modContext.fpStart(Config::CMID_EXEC_PATH.c_str(),
                       Config::CM_CFG_PATH.c_str(),
                       Config::CM_CFG_PATH.c_str());

    //! @todo Load and start Package Manager

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

    modContext.fpStop();

    ReleaseModuleInstance(&modContext);
}

} // namespace CloudManagementClient
