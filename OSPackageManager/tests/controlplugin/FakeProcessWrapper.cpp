/**
 * @file
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#include "FakeProcessWrapper.hpp"
#include "CMLogger.hpp"
#include <algorithm>
#include <cstring>
#include <exception>

//-------------------- FakeProcess ------------------------
FakeProcess::FakeProcess(pid_t pid, std::string strName):
    pid_(pid),
    name_(std::move(strName))
{
}

pid_t FakeProcess::getPid() const
{
    return pid_;
}

bool FakeProcess::isKilled() const
{
    return killed_;
}

const std::string& FakeProcess::getName() const
{
    return name_;
}

void FakeProcess::setKilled(bool bVal)
{
    killed_ = bVal;
}

void FakeProcess::setWaitStatus(IProcessWrapper::EWaitForProcStatus status)
{
    waitStatus_ = status;
}

IProcessWrapper::EWaitForProcStatus FakeProcess::getWaitStatus() const
{
    return waitStatus_;
}

void FakeProcess::signal()
{
    stateChanged_ = true;
    cv_.notify_all();
}

void FakeProcess::wait()
{
    std::unique_lock<std::mutex> lk(mutex_);
    cv_.wait(lk, [this]() -> bool {
        return stateChanged_;
    });
    stateChanged_ = false;
}


//-------------------- FakeProcessWrapper ------------------------
FakeProcessWrapper::FakeProcessWrapper(std::string strDefault):
    defaultProcessName_(std::move(strDefault))
{
}

IProcessWrapper::EWaitForProcStatus FakeProcessWrapper::waitForProcess(pid_t pid)
{
    auto fnPred = [pid] (const FakeProcess& p) {
        return p.getPid() == pid && !p.isKilled();
    };
    
    auto it = std::find_if(processes_.begin(), processes_.end(), fnPred);
    if (it == processes_.end())
        return EWaitForProcStatus::ProcessDoesNotExist;
    
    it->wait();
    return it->getWaitStatus();
}

void FakeProcessWrapper::kill(pid_t pid, EWaitForProcStatus status)
{
    auto fnPred = [pid] (const FakeProcess& p) {
        return p.getPid() == pid && !p.isKilled();
    };
    
    auto it = std::find_if(processes_.begin(), processes_.end(), fnPred);
    if (it == processes_.end())
    {
        CM_LOG_ERROR("Can't find process with pid [%d]", pid);
        return;
    }
    
    it->setWaitStatus(status);
    switch (status)
    {
    case IProcessWrapper::EWaitForProcStatus::ProcessTerminated:
    case IProcessWrapper::EWaitForProcStatus::ProcessExited:
        it->setKilled(true);
        break;
    default:
        break;
    }

    it->signal();
}

void FakeProcessWrapper::kill(pid_t pid)
{
    processedKillCalls_.push_back(pid);
    if (!killCalls_.empty())
    {
        std::function<void(pid_t)> fnKill = killCalls_.back();
        killCalls_.erase(killCalls_.end() - 1);
        fnKill(pid);
        return;
    }
    kill(pid, IProcessWrapper::EWaitForProcStatus::ProcessTerminated);
}

void FakeProcessWrapper::addKillCall(const std::function<void(pid_t)>& fn)
{
    killCalls_.push_back(fn);
}

void FakeProcessWrapper::addForkCall(const std::function<pid_t()>& fn)
{
    forkCalls_.push_back(fn);
}

void FakeProcessWrapper::addExecvCall(const std::function<void(const std::vector<char *>&)>& fn)
{
    execCalls_.push_back(fn);
}


pid_t FakeProcessWrapper::fork()
{
    ++numberTimesForkCalled_;
    if (!forkCalls_.empty())
    {
        std::function<pid_t()> fnFork = forkCalls_.back();
        forkCalls_.erase(forkCalls_.end() - 1);
        return fnFork();
    }
    return createProcess(defaultProcessName_);
}

std::vector<pid_t> FakeProcessWrapper::getRunningProcesses()
{
    std::vector<pid_t> ret;
    for(auto&& process: processes_)
    {
        if (!process.isKilled())
            ret.push_back(process.getPid());
    }
    return ret;
}

#ifdef __APPLE__
bool FakeProcessWrapper::getProcessInfo(pid_t pid, proc_bsdinfo* pProcInfo)
{
    if (!pProcInfo)
        return false;
    
    auto fnPred = [pid] (const FakeProcess& p) {
        return p.getPid() == pid && !p.isKilled();
    };
    
    auto it = std::find_if(processes_.begin(), processes_.end(), fnPred);
    if (it == processes_.end())
        return false;
    
    std::strncpy(pProcInfo->pbi_name, it->getName().c_str(), sizeof(pProcInfo->pbi_name));
    return true;
}
#else
bool FakeProcessWrapper::getProcessInfo(pid_t pid, std::string& exeName)
{
    auto fnPred = [pid] (const FakeProcess& p) {
        return p.getPid() == pid && !p.isKilled();
    };
    
    auto it = std::find_if(processes_.begin(), processes_.end(), fnPred);
    if (it == processes_.end())
        return false;
    
    exeName = it->getName();
    return true;
}
#endif //__APPLE__

pid_t FakeProcessWrapper::createProcess(const std::string& strName)
{
    ++curProcessPid_;
    processes_.emplace_back(curProcessPid_, strName);
    return curProcessPid_;
}

void FakeProcessWrapper::execv(const std::vector<char *>& processArgs)
{
    ++numberTimesExecvCalled_;
    if (!execCalls_.empty())
    {
        std::function<void(const std::vector<char *>&)> fnExec = execCalls_.back();
        execCalls_.erase(execCalls_.end() - 1);
        return fnExec(processArgs);
    }
    throw FakeProcessException();
}

size_t FakeProcessWrapper::getNumberOfExecvCalls() const
{
    return numberTimesExecvCalled_;
}

size_t FakeProcessWrapper::getNumberOfForkCalls() const
{
    return numberTimesForkCalled_;
}

const std::deque<pid_t>& FakeProcessWrapper::getProcessedKillCalls() const
{
    return processedKillCalls_;
}

void FakeProcessWrapper::exit(int nStatus)
{
    ++numberTimesExitCalled_;
}

size_t FakeProcessWrapper::getNumberOfExitCalls() const
{
    return numberTimesExitCalled_;
}
