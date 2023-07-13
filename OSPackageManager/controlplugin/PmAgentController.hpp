/**************************************************************************
 *       Copyright (c) 2023, Cisco Systems, All Rights Reserved
 ***************************************************************************
 *
 *  @file:    PmAgentController.h
 *
 ***************************************************************************
 * @desc Child process controller for Pm Agent
 ***************************************************************************/
#pragma once

#include "PmStatusTypes.hpp"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <filesystem>
#include <atomic>

#include "IProcessWrapper.hpp"

#define PM_AGENT_BINARY "cmpackagemanager"
#define BS_CONFIG_FILE "bs.json"
#define PM_CONFIG_FILE "cm_config.json"
#define INVALID_PID -1
#define RESTART_DELAY_CHRONO 30000

class PmAgentController {
public:
    /**
    * Constructor
    * @param[in] path - Path to the directory containing the package manager agent binary (cmpackagemanager)
    */
    PmAgentController( const std::string& path, const std::string& configPath, std::shared_ptr<IProcessWrapper> pProcessWrapper,  std::chrono::milliseconds restartDelay = std::chrono::milliseconds(RESTART_DELAY_CHRONO) );

    ~PmAgentController();
    /**
    * Starts and monitors the child process
    * @return PM_STATUS
    */
    PM_STATUS start();
    /**
    * Sends a Stop signal to the child process
    * Terminates if unresponsive.
    * @return PM_STATUS
    */
    PM_STATUS stop();
    
    //helper methods used in unit tests
    bool waitMonitorThreadInitialized();
    bool waitForMonitorIteration(size_t nIteration);
    void waitMonitorThreadStopped();
    bool isProcessStartedByPlugin() const;
    void setProcessStartedByPlugin(bool bVal);
private:
    void cleanup();
    void monitorProcess();
    PM_STATUS killIfRunning();
    PM_STATUS startProcess();
    PM_STATUS stopProcess();
    std::chrono::milliseconds getRestartDelay() const;

    std::filesystem::path processPath_;
    std::filesystem::path bsConfigPath_;
    std::filesystem::path pmConfigPath_;
    bool bIsProcessStartedByPlugin_;
    std::mutex mutex_;
    pid_t pid_ = INVALID_PID;
    std::thread threadMonitor_;
    std::shared_ptr<IProcessWrapper> pProcessWrapper_;
    std::condition_variable monitorCondition_;
    std::mutex monitorMtx_;
    mutable std::mutex memberProtectionMtx_;
    std::chrono::milliseconds restartDelay_;
    std::atomic<bool> monitorThreadStarted_ = false;
    std::atomic<size_t> monitorIteration_ = 0;
    std::condition_variable monitorIterationCondition_;
};
