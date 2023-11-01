/**
 * @file  ProxyWatcher.hpp
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#pragma once
#include <SystemConfiguration/SystemConfiguration.h>

class ProxyWatcher
{
public:
    ProxyWatcher();
    virtual ~ProxyWatcher();
    ProxyWatcher(const ProxyWatcher&) = delete;
    
private:
    SCDynamicStoreRef dynamicStore_ = nullptr;
};
