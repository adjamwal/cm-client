/**************************************************************************
 *       Copyright (c) 2023, Cisco Systems, All Rights Reserved
 ***************************************************************************
 *
 *  @file:    IProcessWrapper.hpp
 *
 ***************************************************************************
 * @desc  IWrapper around system calls related to the process creation, monitoring, and killing.
 ***************************************************************************/

#pragma once
#include "IProcessWrapper.hpp"

class ProcessWrapper: public IProcessWrapper
{
public:
    EWaitForProcStatus waitForProcess(pid_t pid) override;
    pid_t fork() override;
    void kill(pid_t pid) override;
    std::vector<pid_t> getRunningProcesses() override;
    bool getProcessInfo(pid_t pid, proc_bsdinfo* pProcInfo) override;
    void execv(const std::vector<char *>& processArgs) override;
    void exit(int nStatus) override;
};
