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
#include <dirent.h>
#define PROC_DIR "/proc/"
#define MAX_LENGTH 1024
#define PATH_DELIMITER "/"
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
    std::vector<pid_t> pids;
    DIR* pDir = opendir(PROC_DIR);
    if (!pDir) {
        CM_LOG_ERROR("Failed to open directory %s", PROC_DIR);
        return pids;
    }
    struct dirent* pDirent = nullptr;
    while ((pDirent = readdir(pDir)) != nullptr) {
        if (strspn(pDirent->d_name, "0123456789") == strlen(pDirent->d_name)) {
            pid_t pid = atoi(pDirent->d_name);
            if (pid > 0) {
                pids.push_back(pid);
            }
        }
    }
    closedir(pDir);
    return pids;
}

bool ProcessWrapper::getProcessInfo(pid_t pid, std::string& exeName)
{
    // Buffer for storing the proc path and executable name
    char szProcPath[MAX_LENGTH] = {0};
    char szBuf[MAX_LENGTH] = {0};

    // Construct the /proc/<pid>/exe path
    snprintf(szProcPath, sizeof(szProcPath), "/proc/%d/exe", pid);

    // Read the symbolic link to get the executable path
    ssize_t len = readlink(szProcPath, szBuf, sizeof(szBuf) - 1);
    if (len == -1) {
        return false;
    }
    szBuf[len] = '\0'; // Null-terminate the path


    // Convert the buffer to a std::string
    std::string strProcessName = szBuf;

    // Extract the process name by finding the last path delimiter
    size_t nPos = strProcessName.find_last_of(PATH_DELIMITER);
    if (nPos != std::string::npos) {
        exeName = strProcessName.substr(nPos + 1);
    } else {
        exeName = strProcessName;
    }

    return true;
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
