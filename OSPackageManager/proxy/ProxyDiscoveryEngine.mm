/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */
#include "ProxyDiscoveryEngine.h"

#include "ScopedGuard.hpp"
#include "PmLogger.hpp"
#include "util/StringUtil.hpp"

#include <limits>

#import <CoreServices/CoreServices.h>
#import <SystemConfiguration/SystemConfiguration.h>
#import <Foundation/Foundation.h>
#import <CFNetwork/CFNetwork.h>

namespace
{

const std::string kPrivateRunLoopMode = "com.cisco.unified_connector.proxy_discovery";

void resultCallback(void * client, CFArrayRef proxies, CFErrorRef error)
{
    CFTypeRef *     resultPtr;
    
    assert( (proxies != NULL) == (error == NULL) );
    
    resultPtr = (CFTypeRef *) client;
    assert( resultPtr != NULL);
    assert(*resultPtr == NULL);
    
    if (error != NULL) {
        *resultPtr = CFRetain(error);
    } else {
        *resultPtr = CFRetain(proxies);
    }
    CFRunLoopStop(CFRunLoopGetCurrent());
}

void CFQRelease(CFTypeRef cf)
{
    if (cf != nullptr) {
        CFRelease(cf);
    }
}

void expandPACProxy(NSURL* testUrl, NSURL* scriptURL, NSMutableArray* retProxies)
{
    CFTypeRef result = nullptr;
    CFStreamClientContext context = {0, &result, nullptr, nullptr, nullptr};
    CFRunLoopSourceRef rls = CFNetworkExecuteProxyAutoConfigurationURL((__bridge CFURLRef)scriptURL, (__bridge CFURLRef)testUrl, resultCallback, &context);
    
    auto guard = util::scoped_guard([&rls]() {
        CFQRelease(rls);
    });
    
    if (rls == nullptr)
    {
        PM_LOG_ERROR("Error occured during CFNetworkExecuteProxyAutoConfigurationURL call: returned CFRunLoopSourceRef is zero.");
        CFQRelease(result);
        return;
    }
    
    NSString* nsPrivateRunLoopMode = [NSString stringWithUTF8String:kPrivateRunLoopMode.c_str()];
    CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, (__bridge CFStringRef)nsPrivateRunLoopMode);
    CFRunLoopRunInMode((__bridge CFStringRef)nsPrivateRunLoopMode, 1.0e10, false);
    CFRunLoopRemoveSource(CFRunLoopGetCurrent(), rls, (__bridge CFStringRef)nsPrivateRunLoopMode);
    
    if (result == nullptr)
    {
        PM_LOG_ERROR("The result of the CFNetworkExecuteProxyAutoConfigurationURL call is zero.");
        return;
    }
    
    if ( CFGetTypeID(result) == CFErrorGetTypeID() ) {
        CFErrorRef cfError = (CFErrorRef)result;
        NSError* err = (__bridge_transfer NSError*)cfError;
        long errorCode = [err code];
        std::string errDomain = util::convertNSStringToStdString([err domain]);
        std::string errDescr = util::convertNSStringToStdString([err localizedDescription]);
        PM_LOG_ERROR("Error during CFNetworkExecuteProxyAutoConfigurationURL call occured: "
            "code: [%ld]. Domain: [%s]. Description: [%s].",
            errorCode, errDomain.c_str(), errDescr.c_str());
        return;
    } else if ( CFGetTypeID(result) == CFArrayGetTypeID() ) {
        NSArray* expandedProxies = (__bridge_transfer NSArray*)result;
        for (NSDictionary* dictionary in expandedProxies) {
            [retProxies addObject:dictionary];
        }
    } else {
        PM_LOG_ERROR("Unknown error occured during CFNetworkExecuteProxyAutoConfigurationURL call.");
        CFQRelease(result);
        return;
    }
}

NSArray* expandPACProxies(NSURL* testUrl, NSArray* inputProxies)
{
    NSMutableArray* retProxies = [[NSMutableArray alloc] init];
    for (NSDictionary* dictionary in inputProxies) {
        NSString* proxyType = [dictionary objectForKey: (__bridge NSString*)kCFProxyTypeKey];
        if ([proxyType isEqualToString: (__bridge NSString*)kCFProxyTypeAutoConfigurationURL])
        {
            NSURL* scriptURL = [dictionary objectForKey: (__bridge NSURL*)kCFProxyAutoConfigurationURLKey];
            if (scriptURL == nullptr)
                continue;
            expandPACProxy(testUrl, scriptURL, retProxies);
        }
        else
        {
            [retProxies addObject:dictionary];
        }
       
    }
    return retProxies;
}
} //unnamed namespace

std::list<PmProxy> ProxyDiscoveryEngine::getProxiesInternal(const std::string& testUrlStr, const std::string &pacUrlStr)
{
    std::list<PmProxy> proxyList;
    
    NSDictionary* proxySettings =
        (__bridge_transfer NSDictionary*)SCDynamicStoreCopyProxies(nullptr);
    
    if (proxySettings == nullptr)
    {
        PM_LOG_WARNING("The SCDynamicStoreCopyProxies call returned zero dictionary as a proxy settings.");
        return proxyList;
    }
    
    NSString* nsTestUrlStr = [NSString stringWithUTF8String:testUrlStr.c_str()];
    NSURL* testUrl = [NSURL URLWithString: nsTestUrlStr];
    
    NSMutableArray* proxies = [[NSMutableArray alloc] init];
    if (!pacUrlStr.empty())
    {
        PM_LOG_DEBUG("Pac url is provided for the proxy discovery: %s", pacUrlStr.c_str());
        NSString* nsPacUrlStr = [NSString stringWithUTF8String:pacUrlStr.c_str()];
        NSURL* pacUrl = [NSURL URLWithString: nsPacUrlStr];
        expandPACProxy(testUrl, pacUrl, proxies);
    }
    
    NSArray* systemProxies = (__bridge_transfer NSArray*)CFNetworkCopyProxiesForURL((__bridge CFURLRef)testUrl, (__bridge CFDictionaryRef)proxySettings);
    
    NSArray* expandedProxies = expandPACProxies(testUrl, systemProxies);
    [proxies addObjectsFromArray:expandedProxies];
    
    for (NSDictionary* dictionary in proxies) {
        //TODO: We might need to extend PmProxy type to include
        //credentials fields: kCFProxyUsernameKey, kCFProxyPasswordKey.
        NSString* nsStrProxyType = [dictionary objectForKey:(__bridge NSString*)kCFProxyTypeKey];
        std::string proxyType = util::convertNSStringToStdString(nsStrProxyType);
        
        NSString* nsStrHost = [dictionary objectForKey:(__bridge NSString*)kCFProxyHostNameKey];
        
        NSString* nsStrUrl = [dictionary objectForKey:(__bridge NSString*)kCFProxyAutoConfigurationURLKey];
        
        std::string url;
        if (nsStrUrl == nullptr)
        {
            if (nsStrHost != nullptr)
            {
                url = util::convertNSStringToStdString(nsStrHost);
            }
            else
            {
                PM_LOG_DEBUG("Unable to determine proxy URL: both kCFProxyHostNameKey and kCFProxyHostNameKey are empty in the dictionary.");
                continue;
            }
        }
        else
        {
            url = util::convertNSStringToStdString(nsStrUrl);
        }
        
        NSNumber* nsPort = [dictionary objectForKey:(__bridge NSNumber*)kCFProxyPortNumberKey];
        uint32_t port = [nsPort intValue];
        
        PmProxy proxy{url, port, proxyType};
        proxyList.push_back(proxy);
    }
    
    return proxyList;
}

void ProxyDiscoveryEngine::addObserver(IProxyObserver* pObserver)
{
    m_observers.push_back(pObserver);
}

void ProxyDiscoveryEngine::requestProxiesAsync(const std::string& testUrl, const std::string &pacUrlStr, const std::string& guid)
{
    if (m_thread && m_thread->joinable())
    {
        //wait for the previous discovery completed.
        m_thread->join();
    }
    m_thread = std::make_shared<std::thread>([this, testUrl, pacUrlStr, guid](){
        std::list<PmProxy> proxies = getProxiesInternal(testUrl, pacUrlStr);
        notifyObservers(proxies, guid);
    });
}

void ProxyDiscoveryEngine::waitPrevOpCompleted()
{
    if (m_thread && m_thread->joinable())
    {
        //wait for the previous discovery completed.
        m_thread->join();
    }
}

void ProxyDiscoveryEngine::notifyObservers(const std::list<PmProxy>& proxies, const std::string& guid)
{
    for(auto* pObserver: m_observers)
    {
        if (pObserver) {
            //TODO: maybe we need to call observer on the main thread?
            pObserver->updateProxyList(proxies, guid);
        }
    }
}

std::list<PmProxy> ProxyDiscoveryEngine::getProxies(const std::string& testUrl, const std::string &pacUrl)
{
    //We need to call getProxiesInternal in a separate thread even for the
    //synchronous call since expandPACProxy function is running event loop.
    //if we do it in separate thread event loop of this separate thread will be run.
    //Event loop of the main thread is not touched.
    if (m_threadSync)
    {
        //wait for the previous discovery completed.
        m_threadSync->join();
    }
    std::list<PmProxy> proxies;
    m_threadSync = std::make_shared<std::thread>([this, &testUrl, &pacUrl, &proxies](){
        proxies = getProxiesInternal(testUrl, pacUrl);
    });
    //Wait for the thread to make call synchronous
    m_threadSync->join();
    return proxies;
}

ProxyDiscoveryEngine::~ProxyDiscoveryEngine()
{
    //wait for the threads completion
    if (m_thread && m_thread->joinable())
        m_thread->join();
    
    if (m_threadSync && m_threadSync->joinable())
        m_threadSync->join();
}
