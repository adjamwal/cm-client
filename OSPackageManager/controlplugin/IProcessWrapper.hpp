/**************************************************************************
 *       Copyright (c) 2023, Cisco Systems, All Rights Reserved
 ***************************************************************************
 *
 *  @file:    IProcessWrapper.hpp
 *
 ***************************************************************************
 * @desc  Interface which contains methods that are wrappers around system calls.
 ***************************************************************************/
#pragma once
#include <sys/types.h>
#include <sys/proc_info.h>
#include <system_error>
#include <vector>

class IProcessWrapper
{
public:
    
    enum class EWaitForProcStatus {
        ProcessTerminated = 0,
        ProcessExited,
        ImpossibleError,
        ProcessNotAChild,
        ProcessDoesNotExist,
        WaitInterruptedBySignal,
        UnknownStatus,
    };
    
    virtual ~IProcessWrapper() = default;
    
    virtual EWaitForProcStatus waitForProcess(pid_t pid) = 0;
    virtual pid_t fork() = 0;
    virtual void kill(pid_t pid) = 0;
    virtual std::vector<pid_t> getRunningProcesses() = 0;
    virtual bool getProcessInfo(pid_t pid, proc_bsdinfo* pProcInfo) = 0;
    virtual void execv(const std::vector<char *>& processArgs) = 0;
    virtual void exit(int nStatus) = 0;
};
