/**
 * @file  ProxyWatcher.cpp
 *
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved
 */


#include "ProxyWatcher.hpp"
#include "crashpad/CrashpadTuner.h"

#include <chrono>
#include <iostream>

using namespace std::chrono_literals;

ProxyWatcher::ProxyWatcher() : m_stop(false)
{
    m_thread = std::make_shared<std::thread>([this]() {
        while (!m_stop.load(std::memory_order_acquire)) {
            CrashpadTuner::getInstance()->startProxyDiscoveryAsync();
            std::this_thread::sleep_for(60s);
        }
    });
}

ProxyWatcher::~ProxyWatcher()
{
    m_stop.store(true, std::memory_order_release);
    if (m_thread && m_thread->joinable()) {
        //wait for the previous discovery completed.
        m_thread->join();
    }
}

