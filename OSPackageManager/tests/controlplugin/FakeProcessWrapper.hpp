/**************************************************************************
 *       Copyright (c) 2023, Cisco Systems, All Rights Reserved
 ***************************************************************************
 *
 *  @file:    FakeProcessWrapper.hpp
 *
 ***************************************************************************
 * @desc  Implementation of the IProcessWrapper that is going to be used in Unit Tests.
 ***************************************************************************/
#pragma once
#include "IProcessWrapper.hpp"
#include <mutex>
#include <condition_variable>
#include <deque>
#include <functional>
#include <exception>
#include <atomic>

class FakeProcess
{
public:
    FakeProcess(pid_t pid, std::string strName);
    pid_t getPid() const;
    bool isKilled() const;
    const std::string& getName() const;
    IProcessWrapper::EWaitForProcStatus getWaitStatus() const;
    void setKilled(bool bVal);
    void setWaitStatus(IProcessWrapper::EWaitForProcStatus status);
    void signal();
    void wait();
    
private:
    std::condition_variable cv_;
    std::mutex mutex_;
    std::atomic<pid_t> pid_;
    std::atomic<bool> killed_ = false;
    std::string name_;
    std::atomic<IProcessWrapper::EWaitForProcStatus> waitStatus_ = IProcessWrapper::EWaitForProcStatus::UnknownStatus;
    std::atomic<bool> stateChanged_ = false;
};

class FakeProcessException: public std::exception
{
};

class FakeProcessWrapper: public IProcessWrapper
{
public:
    explicit FakeProcessWrapper(std::string strDefault);
    EWaitForProcStatus waitForProcess(pid_t pid) override;
    pid_t fork() override;
    void kill(pid_t pid) override;
    std::vector<pid_t> getRunningProcesses() override;
#ifdef __APPLE__
    bool getProcessInfo(pid_t pid, proc_bsdinfo* pProcInfo) override;
#else
    bool getProcessInfo(pid_t pid, void* pProcInfo) override;
#endif
    //should throw FakeProcessException if no error
    void execv(const std::vector<char *>& processArgs) override;
    void exit(int nStatus) override;

    //Test helper methods
    void kill(pid_t pid, EWaitForProcStatus status);
    void addKillCall(const std::function<void(pid_t)>& fn);
    void addForkCall(const std::function<pid_t()>& fn);
    void addExecvCall(const std::function<void(const std::vector<char *>&)>& fn);
    pid_t createProcess(const std::string& strName);
    //TODO: protect access with mutexes?
    size_t getNumberOfExecvCalls() const;
    size_t getNumberOfForkCalls() const;
    size_t getNumberOfExitCalls() const;
    const std::deque<pid_t>& getProcessedKillCalls() const;
private:
    std::deque<FakeProcess> processes_;
    pid_t curProcessPid_ = 0;
    
    std::deque<std::function<void(pid_t)>> killCalls_;
    std::deque<std::function<pid_t()>> forkCalls_;
    std::deque<std::function<void(const std::vector<char *>&)>> execCalls_;
    std::string defaultProcessName_;
    size_t numberTimesExecvCalled_ = 0;
    std::deque<pid_t> processedKillCalls_;
    size_t numberTimesForkCalled_ = 0;
    size_t numberTimesExitCalled_ = 0;
};
