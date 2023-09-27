/**************************************************************************
 *       Copyright (c) 2023, Cisco Systems, All Rights Reserved
 ***************************************************************************
 *
 *  @file:    ThreadTimer.hpp
 *
 ***************************************************************************
 * @desc Implementation of the timer based on the std::thread.
 ***************************************************************************/
#pragma once

#include <atomic>
#include <thread>
#include <memory>

namespace util
{

class ThreadTimer: public std::enable_shared_from_this<ThreadTimer>
{
public:
    
    ThreadTimer() = default;
    ~ThreadTimer();
    ThreadTimer(const ThreadTimer&) = delete;
    ThreadTimer(ThreadTimer&&) = delete;
    ThreadTimer& operator =(const ThreadTimer&) = delete;
    ThreadTimer& operator =(ThreadTimer&&) = delete;
    
    template<typename TFunc, typename Rep, typename Period>
    void start(TFunc func, const std::chrono::duration<Rep, Period>& interval);
    
    void stop();
    void setOneTime(bool bVal);
    void setExecuteImmediately(bool bVal);
    bool shouldExecuteImmediately() const;
    
private:
    std::atomic<bool> bActive_ = false;
    std::atomic<bool> bOneTime_ = false;
    std::atomic<bool> bExecuteImmediately_ = true;
};

ThreadTimer::~ThreadTimer()
{
    stop();
}

template<typename TFunc, typename Rep, typename Period>
void ThreadTimer::start(TFunc func, const std::chrono::duration<Rep, Period>& interval)
{
    bActive_ = true;
    std::weak_ptr<ThreadTimer> weakThis = weak_from_this();
    std::thread t([weakThis, func, interval]() {
        {
            auto sharedThis = weakThis.lock();
            if (!sharedThis)
                return;
            if (!sharedThis->shouldExecuteImmediately())
            {
                sharedThis.reset();
                std::this_thread::sleep_for(interval);

                {
                    sharedThis = weakThis.lock();
                    if(!sharedThis || !sharedThis->bActive_)
                        return;
                }
            }
        }
        do {
            {
                auto sharedThis = weakThis.lock();
                if (!sharedThis)
                    break;
                func();
                if (sharedThis->bOneTime_)
                    break;
                
                if(!sharedThis->bActive_)
                    break;
            }

            std::this_thread::sleep_for(interval);
            
            {
                auto sharedThis = weakThis.lock();
                if(!sharedThis || !sharedThis->bActive_)
                    break;
            }
            
        } while(true);
    });
    t.detach();
}

void ThreadTimer::stop()
{
    bActive_ = false;
}

void ThreadTimer::setOneTime(bool bVal)
{
    bOneTime_ = bVal;
}

void ThreadTimer::setExecuteImmediately(bool bVal)
{
    bExecuteImmediately_ = bVal;
}

bool ThreadTimer::shouldExecuteImmediately() const
{
    return bExecuteImmediately_ && !bOneTime_;
}

} //util namespace
