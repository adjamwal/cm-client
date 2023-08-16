#pragma once

#include "PackageManager/PmTypes.h"

#include <list>

class IProxyObserver
{
public:
    virtual void updateProxyList(const std::list<PmProxy>& proxies, const std::string& guid) = 0;
};

class IProxyDiscoveryEngine {
public:
    virtual ~IProxyDiscoveryEngine() = default;
    virtual void addObserver(IProxyObserver* pObserver) = 0;
    virtual void requestProxiesAsync(const std::string& testUrl, const std::string &pacUrl, const std::string& guid) = 0;
    virtual void waitPrevOpCompleted() = 0;
    virtual std::list<PmProxy> getProxies(const std::string& testUrl, const std::string &pacUrl) = 0;
};
