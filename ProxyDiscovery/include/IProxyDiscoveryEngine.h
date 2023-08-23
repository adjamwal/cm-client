#pragma once

#include "ProxyRecord.h"
#include "ProxyDef.h"

#include <list>
#include <memory>

namespace proxy
{

class IProxyObserver
{
public:
    virtual void updateProxyList(const std::list<ProxyRecord>& proxies, const std::string& guid) = 0;
};

/**
 * @brief An interface which aim is to perform available proxy settings discovery.
 */
class PROXY_DISCOVERY_MODULE_API IProxyDiscoveryEngine {
public:
    virtual ~IProxyDiscoveryEngine() = default;
    virtual void addObserver(IProxyObserver* pObserver) = 0;
    virtual void requestProxiesAsync(const std::string& testUrl, const std::string &pacUrl, const std::string& guid) = 0;
    virtual void waitPrevOpCompleted() = 0;
    virtual std::list<ProxyRecord> getProxies(const std::string& testUrl, const std::string &pacUrl) = 0;
};

std::unique_ptr<IProxyDiscoveryEngine>  PROXY_DISCOVERY_MODULE_API createProxyEngine();

} //proxy
