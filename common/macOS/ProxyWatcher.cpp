/**
 * @file  ProxyWatcher.cpp
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */


#include "ProxyWatcher.hpp"
#include "crashpad/CrashpadTuner.h"

#include <iostream>

static void internalProxyConfigChangedCallback(SCDynamicStoreRef /*store*/, CFArrayRef /*changedKeys*/, void* /*info*/)
{
    CrashpadTuner::getInstance()->startProxyDiscoveryAsync();
}

ProxyWatcher::ProxyWatcher()
{
    //Initial discovery on startup
    internalProxyConfigChangedCallback(nullptr, nullptr, nullptr);
    
    SCDynamicStoreContext   context = {0, NULL, NULL, NULL, NULL};
    dynamicStore_ = SCDynamicStoreCreate(NULL, CFSTR("proxyObserver"), internalProxyConfigChangedCallback, &context);
    if (dynamicStore_) {
        CFStringRef patterns = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL, kSCDynamicStoreDomainSetup,
                                                                           kSCCompAnyRegex, kSCEntNetProxies);
        CFArrayRef patternList = CFArrayCreate(NULL, (const void **) &patterns, 1, &kCFTypeArrayCallBacks);
        SCDynamicStoreSetNotificationKeys(dynamicStore_, NULL ,patternList);
        CFRelease(patterns);
        CFRelease(patternList);
        
        SCDynamicStoreSetDispatchQueue(dynamicStore_, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0));
    }
}

ProxyWatcher::~ProxyWatcher()
{
    if (dynamicStore_)
        CFRelease(dynamicStore_);
}

