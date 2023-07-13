/**
 * @file
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#include "PmAgentController.hpp"
#include "ExecutionError.hpp"
#include "CMLogger.hpp"
#include "util/ScopedGuard.hpp"
#include <iostream>
#include <vector>
#include <system_error>

using namespace std::chrono_literals;

namespace
{
    constexpr std::chrono::microseconds kWaitDelay = 1000ms;
}

PmAgentController::PmAgentController( const std::string& path, const std::string& configPath, std::shared_ptr<IProcessWrapper> pProcessWrapper, std::chrono::milliseconds restartDelay) :
      processPath_( std::filesystem::path( path ) / PM_AGENT_BINARY )
    , bsConfigPath_( std::filesystem::path( configPath ) / BS_CONFIG_FILE )
    , pmConfigPath_( std::filesystem::path( configPath ) / PM_CONFIG_FILE )
    , bIsProcessStartedByPlugin_( false )
    , pProcessWrapper_(std::move(pProcessWrapper))
    , restartDelay_(restartDelay)
{
    if( path.empty() ) {
        throw std::invalid_argument( "AgentController basepath has not been set" );
    }
}

PmAgentController::~PmAgentController()
{
    stop();
    cleanup();
}

PM_STATUS PmAgentController::start()
{
    //check if previous process is running
    // and kill it.
    if( PM_STATUS::PM_OK != killIfRunning() )
    {
        CM_LOG_ERROR( "Could not kill previous running instance" );
        return PM_STATUS::PM_FAIL;
    }

    //start process.
    if( PM_STATUS::PM_OK != startProcess() )
    {
        CM_LOG_ERROR( "Could not start the process." );
        return PM_STATUS::PM_FAIL;
    }

    setProcessStartedByPlugin(true);
    CM_LOG_DEBUG( "Process successfully launched." );

    //launch thread to monitor process.
    threadMonitor_ = std::thread( &PmAgentController::monitorProcess, this );
    return PM_STATUS::PM_OK;
}

PM_STATUS PmAgentController::stop()
{
    bool bWaitForThread = isProcessStartedByPlugin();
    {
        std::lock_guard<std::mutex> lock( mutex_ );
        if( isProcessStartedByPlugin() )
        {
            setProcessStartedByPlugin(false);
            if( PM_STATUS::PM_OK != stopProcess() ) {
                CM_LOG_ERROR( "Could not stop the process." );
                return PM_STATUS::PM_FAIL;
            }
            CM_LOG_DEBUG( "Process successfully stopped." );
        }
    }
    //Wait for monitor thread to exit
    if(  threadMonitor_.joinable() && bWaitForThread )
    {
        threadMonitor_.join();
    }
    return PM_STATUS::PM_OK;
}

void PmAgentController::monitorProcess()
{
    {
        std::lock_guard lk(monitorMtx_);
        monitorThreadStarted_ = true;
    }
    monitorCondition_.notify_all();
    while(true)
    {
        IProcessWrapper::EWaitForProcStatus waitCode = pProcessWrapper_->waitForProcess(pid_);
        {
            std::lock_guard<std::mutex> lock( mutex_ );
            util::scoped_guard([this]() {
                ++monitorIteration_;
            });
            if (waitCode == IProcessWrapper::EWaitForProcStatus::ImpossibleError)
            {
                CM_LOG_DEBUG("The waitForProcess generated impossible error. Stop restarting the cmpackagemanager process");
                break;
            }
            if (waitCode == IProcessWrapper::EWaitForProcStatus::ProcessNotAChild)
            {
                try
                {
                    pProcessWrapper_->kill(pid_);
                    CM_LOG_DEBUG( "Process name = [%s] with pid = [%d] terminated.", PM_AGENT_BINARY, pid_ );
                }
                catch(std::system_error& e)
                {
                    CM_LOG_ERROR( "Process name = [%s] failed to terminate. Error code: [%d], meaning: [%s]", PM_AGENT_BINARY, e.code().value(), e.code().message().c_str() );
                }
            }
           
            CM_LOG_DEBUG( "Child process signalled..." );
            //check if the process was terminated by us
            //and if yes, stop monitoring.
            if( !isProcessStartedByPlugin() )
            {
                break;
            }
            
            CM_LOG_WARNING( "Child process terminated. Starting it again." );
            pid_ = INVALID_PID;
            std::this_thread::sleep_for( getRestartDelay() );
            startProcess();
        }
        monitorIterationCondition_.notify_all();
    }
    monitorIterationCondition_.notify_all();
    monitorThreadStarted_ = false;
    CM_LOG_DEBUG( "Exiting monitor thread" );
}

void PmAgentController::cleanup()
{
    pid_ = -1;
}

PM_STATUS PmAgentController::killIfRunning()
{
    std::vector<pid_t> processIDs = pProcessWrapper_->getRunningProcesses();
    for (auto&& pid: processIDs)
    {
        proc_bsdinfo procInfo;
        if (pProcessWrapper_->getProcessInfo(pid, &procInfo))
        {
            if( PM_AGENT_BINARY == std::string(procInfo.pbi_name ) ) {
                
                try
                {
                    pProcessWrapper_->kill(pid);
                    CM_LOG_DEBUG( "Process name = [%s] with pid = [%d] terminated.", PM_AGENT_BINARY, pid );
                    break;
                }
                catch(std::system_error& e)
                {
                    CM_LOG_ERROR( "Process name = [%s] failed to terminate. Error code: [%d], meaning: [%s]", procInfo.pbi_name, e.code().value(), e.code().message().c_str() );
                }
                return PM_STATUS::PM_ERROR;
            }
        }
    }
    return PM_STATUS::PM_OK;
}

PM_STATUS PmAgentController::startProcess()
{
    PM_STATUS status = PM_STATUS::PM_ERROR;

    // TODO Verify Codesign
    // Will be removed once Codesign verification is implemented.
    status = PM_STATUS::PM_OK;

    if ( INVALID_PID != pid_ ) {
        CM_LOG_DEBUG( "Process is still running, pid = %d", pid_ );
        return PM_STATUS::PM_ERROR;
    }

    std::vector<char *> processArgs = {
        strdup(processPath_.c_str()),
        strdup("--bootstrap"),
        strdup(bsConfigPath_.c_str()),
        strdup("--config-file"),
        strdup(pmConfigPath_.c_str()),
        NULL
    };
    try
    {
        pid_ = pProcessWrapper_->fork();
        if (pid_ == 0)
        {
            // Child
            pProcessWrapper_->execv(processArgs);
        }
        else
        {
            // Parent
            CM_LOG_DEBUG( "Starting process %s with Pid %d", processPath_.c_str(), pid_ );
        }
    }
    catch(ExecutionError& ee)
    {
        CM_LOG_ERROR( "execv failed, Failed to start Agent, error code: [%d], message: [%s]", ee.code().value(), ee.code().message().c_str() );
        pProcessWrapper_->exit(ee.code().value());
    }
    catch(std::system_error& e)
    {
        CM_LOG_ERROR( "Child process creation failed, error code: %d. That means following: %s", e.code().value(), e.code().message().c_str());
        status = PM_STATUS::PM_ERROR;
        pid_ = INVALID_PID;
    }
    return status;
}

PM_STATUS PmAgentController::stopProcess()
{
    if( INVALID_PID == pid_ ) {
        CM_LOG_WARNING( "Unable to stop, process is not running since pid_ member is equal to the invalid pid value." );
        return PM_STATUS::PM_OK;
    }
    
    if (pid_ <= 0)
    {
        CM_LOG_WARNING("Wrong pid is provided to the kill function: %d", pid_);
        return PM_STATUS::PM_OK;
    }

    try
    {
        pProcessWrapper_->kill(pid_);
        CM_LOG_DEBUG( "Stopped the process %s with Pid %d", processPath_.c_str(), pid_ );
        pid_ = INVALID_PID;
        return PM_STATUS::PM_OK;
    }
    catch (std::system_error& e)
    {
        CM_LOG_ERROR( "Filed to kill child process with pid: %d. Following error produced, code: %d, meaning: %s", pid_, e.code().value(), e.code().message().c_str());
    }
    return PM_STATUS::PM_ERROR;
}

bool PmAgentController::waitMonitorThreadInitialized()
{
    if (monitorThreadStarted_)
        return true;
    
    std::unique_lock lk(monitorMtx_);
    return monitorCondition_.wait_for(lk, kWaitDelay, [this]() -> bool {
        return monitorThreadStarted_;
    });
}

bool PmAgentController::waitForMonitorIteration(size_t nIteration)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (monitorIteration_ >= nIteration)
        return true;
    
    if (!monitorThreadStarted_)
        return true;

    return monitorIterationCondition_.wait_for(lock, kWaitDelay, [this, nIteration] () -> bool {
        return monitorIteration_ >= nIteration;
    });
}

void PmAgentController::waitMonitorThreadStopped()
{
    if (!monitorThreadStarted_)
        return;

    //Wait for monitor thread to exit
    if( threadMonitor_.joinable() )
    {
        threadMonitor_.join();
    }
}

bool PmAgentController::isProcessStartedByPlugin() const
{
    std::lock_guard<std::mutex> lock( memberProtectionMtx_ );
    return bIsProcessStartedByPlugin_;
}

void PmAgentController::setProcessStartedByPlugin(bool bVal)
{
    std::lock_guard<std::mutex> lock( memberProtectionMtx_ );
    bIsProcessStartedByPlugin_ = bVal;
}

std::chrono::milliseconds PmAgentController::getRestartDelay() const
{
    using namespace std::chrono_literals;
    std::lock_guard<std::mutex> lock( memberProtectionMtx_ );
    return bIsProcessStartedByPlugin_ ? restartDelay_ : 0ms;
}
