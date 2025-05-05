/**
 * @file  ProxyWatcher.hpp
 *
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved
 */

#pragma once

#include <thread>
#include <memory>
#include <atomic>

class ProxyWatcher
{
public:
    ProxyWatcher();
    virtual ~ProxyWatcher();
    ProxyWatcher(const ProxyWatcher&) = delete;
    
private:
    std::shared_ptr<std::thread> m_thread;
    std::atomic<bool> m_stop;
};
