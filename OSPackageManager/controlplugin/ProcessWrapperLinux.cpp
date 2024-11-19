/**
 * @file
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#include "ProcessWrapper.hpp"

#include <ExecutionError.hpp>
#include "CMLogger.hpp"
#include <cerrno>
#include <cstring>
#include <limits>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <system_error>

namespace
{
    const pid_t kInvalidPid = -1;

    std::error_code errno_to_error_code(int errnoValue) {
        static const std::error_category& errnoCategory = std::generic_category();
        return std::error_code(errnoValue, errnoCategory);
    }
}

ProcessWrapper::EWaitForProcStatus ProcessWrapper::waitForProcess(pid_t pid)
{
    if (pid <= kInvalidPid)
    {
        return EWaitForProcStatus::UnknownStatus;
    }
    
    int childStatus = 0;
    EWaitForProcStatus status = EWaitForProcStatus::UnknownStatus;
    pid_t pidRes = waitpid(pid, &childStatus, 0);
    if (pidRes == 0)
    {
        CM_LOG_ERROR("The waitpid souldn't return 0 if third parameter is 0");
        status = EWaitForProcStatus::ImpossibleError;
    }
    else if (pidRes == kInvalidPid)
    {
        int nCurError = errno;
        CM_LOG_ERROR("The waitpid generated an error with the following code %d. That means following: %s",
                     nCurError,  std::strerror(nCurError));
        if (nCurError == ECHILD)
        {
            //Check if process exists
            if (pid > 0 && ::kill(pid, 0) == 0)
            {
                CM_LOG_ERROR("Process with pid [%d] exists but we can't wait for it. This could happen if process in not in our child process or if current process ignores SIGCHLD signal.", pid);
                status = EWaitForProcStatus::ProcessNotAChild;
            }
            else
            {
                CM_LOG_ERROR("Process with pid: [%d] does not exist.", pid);
                status = EWaitForProcStatus::ProcessDoesNotExist;
            }
        }
        else if (nCurError == EINTR)
        {
            status = EWaitForProcStatus::WaitInterruptedBySignal;
        }
        else
        {
            CM_LOG_ERROR("Unexpected error code generated after waitpid call: %d", nCurError);
        }
        return status;
    }
    
    //pidRes > 0, pid of the child process is returned
    return (WIFEXITED(childStatus) ? EWaitForProcStatus::ProcessExited : EWaitForProcStatus::ProcessTerminated);
}

pid_t ProcessWrapper::fork()
{
    pid_t pid = ::fork();
    if (pid == kInvalidPid)
    {
        std::error_code ec = errno_to_error_code(errno);
        throw std::system_error(ec);
    }
    return pid;
}

void ProcessWrapper::kill(pid_t pid)
{
    if( 0 != ::kill( pid, SIGTERM ) ) {
        std::error_code ec = errno_to_error_code(errno);
        throw std::system_error(ec);
        
    }
}

std::vector<pid_t> ProcessWrapper::getRunningProcesses()
{
    std::vector<pid_t> ret;

    /** @todo Implement for Linux */

    return ret;
}

bool ProcessWrapper::getProcessInfo(pid_t pid, void* pProcInfo)
{
    if (pProcInfo == nullptr)
    {
        return false;
    }
    return false;
}

void ProcessWrapper::execv(const std::vector<char *>& processArgs)
{
    if (0 != ::execv( processArgs[0], processArgs.data())) {
        std::error_code ec = errno_to_error_code(errno);
        throw ExecutionError(ec);
    }
}

void ProcessWrapper::exit(int nStatus)
{
    ::exit(nStatus);
}
