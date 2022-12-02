/**
 * @file
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "Daemon.hpp"

#include "ComponentLoader/CMIDLoader.hpp"
#include "Configuration/Config.hpp"
#include "Logger/CMLogFile.hpp"
#include "Logger/CMLogger.hpp"

#include <sys/stat.h>

#include <chrono>
#include <iostream>

namespace CloudManagementClient
{
//TODO : finalise log file name
const std::string logFileName = "csc_cms.log";

Daemon::Daemon()
    : cmidLoader_ { std::make_unique<ComponentLoader::CMIDLoader>() },
      config_ { std::make_unique<CloudManagementConfiguration::Config>()},
      m_logFile( nullptr ),
      m_logger( nullptr )
{
    m_logFile = std::unique_ptr<ICMLogFile>( new CMLogFile() );
    const auto logFilePath = CloudManagementConfiguration::Config::CM_LOG_PATH + logFileName;
    m_logFile->Init( logFilePath.c_str() );
    m_logger = std::unique_ptr<CMLogger>( new CMLogger( *m_logFile ) );
    SetCMLogger( m_logger.get() );
}

void Daemon::init()
{
    config_->load();
}

void Daemon::start()
{
    CM_LOG_DEBUG("Starting cloud management");
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

    cmidLoader_->start();
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

    cmidLoader_->stop();
}

} // namespace CloudManagementClient
