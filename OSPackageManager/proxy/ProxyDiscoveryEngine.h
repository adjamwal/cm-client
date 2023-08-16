#pragma once
#include "IProxyDiscoveryEngine.h"

#include "PackageManager/PmTypes.h"

#include <deque>
#include <memory>
#include <thread>

/**
* @brief A class that performs available proxy settings discovery.
*/
class ProxyDiscoveryEngine: public IProxyDiscoveryEngine {
public:
    ~ProxyDiscoveryEngine();
    ProxyDiscoveryEngine() = default;
    ProxyDiscoveryEngine(const ProxyDiscoveryEngine&) = delete;
    ProxyDiscoveryEngine(ProxyDiscoveryEngine&&) = delete;
    ProxyDiscoveryEngine& operator = (const ProxyDiscoveryEngine&) = delete;
    ProxyDiscoveryEngine& operator = (ProxyDiscoveryEngine&&) = delete;

    void addObserver(IProxyObserver* pObserver) override;
    void requestProxiesAsync(const std::string& testUrl, const std::string &pacUrl, const std::string& guid) override;
    std::list<PmProxy> getProxies(const std::string& testUrl, const std::string &pacUrl) override;
    void waitPrevOpCompleted() override;
    
private:
    std::list<PmProxy> getProxiesInternal(const std::string& testUrl, const std::string &pacUrlStr);
    void notifyObservers(const std::list<PmProxy>& proxies, const std::string& guid);
    std::deque<IProxyObserver*> m_observers;
    std::shared_ptr<std::thread> m_thread;
    std::shared_ptr<std::thread> m_threadSync;
};
