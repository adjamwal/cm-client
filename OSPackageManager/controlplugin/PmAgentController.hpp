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
#include <string>
#include <filesystem>

#define PM_AGENT_BINARY "cmpackagemanager"
#define BS_CONFIG_FILE "bs.json"
#define PM_CONFIG_FILE "cm_config.json"
#define INVALID_PID -1

class PmAgentController {
public:
    /**
    * Constructor
    * @param[in] path - Path to the directory containing the package manager agent binary (cmpackagemanager)
    */
    PmAgentController( const std::string& path, const std::string& configPath );

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
private:
    enum eProcStatus {
        eProcess_Terminated = 0,
        eProcess_Active
    };
    void cleanup();
    void monitorProcess();
    PM_STATUS killIfRunning();
    PM_STATUS startProcess();
    PM_STATUS stopProcess();
    eProcStatus waitForProcess();

    std::filesystem::path processPath_;
    std::filesystem::path bsConfigPath_;
    std::filesystem::path pmConfigPath_;
    bool bIsProcessStartedByPlugin_;
    std::mutex mutex_;
    pid_t pid_ = INVALID_PID;
    std::thread threadMonitor_;

};
