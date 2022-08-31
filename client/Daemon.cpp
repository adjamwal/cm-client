/**
 * @file
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "Daemon.hpp"

#include "ComponentLoader/CMIDLoader.hpp"
#include "Configuration/Config.hpp"

#include "cmid/CMIDAgentController.h"
#include "cmid/CMIDLogger.h"

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
    try {
        const std::string strLogFilePath = Config::CM_LOG_PATH + "csc_cmid_control_plugin.log";
        //Initialise logger
        CMID_LOG_INIT(strLogFilePath);
    }
    catch(const std::exception& rExcep) {
        std::cerr << "Failed to Initialize logger: " << rExcep.what() << std::endl;
        return;
    }
}

void Daemon::mainTask()
{
    using namespace cmid;

    /* Load and start CMID controller... */
    auto cmid_controller = new ComponentLoader::CMIDLoader{
                    std::make_unique<CCMIDAgentController>(Config::CMID_EXEC_PATH,
                                                           Config::CM_CFG_PATH) };
    cmid_controller->load();

    //! @todo Load and start Package Manager

    //! TODO: Just busy wait??
    //!
    //! Change as needed...
    while(this->isRunning_) {
        using namespace std;
        using namespace std::chrono_literals;

        umask(0077);

        cout << "Do things here!" << endl;

        //auto start = chrono::high_resolution_clock::now();
        (void) chrono::high_resolution_clock::now();
        this_thread::sleep_for(1000ms);
        //auto end = chrono::high_resolution_clock::now();
        (void) chrono::high_resolution_clock::now();
    }
}

} // namespace CloudManagementClient
