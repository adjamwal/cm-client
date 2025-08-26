/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */
#pragma once

#include "PackageManager/IPmPlatformConfiguration.h"
#include "CMIDAPIProxyAbstract.hpp"
#include "cmid/CMIDAPI.h"
#include "ProxyDiscovery/IProxyDiscoveryEngine.h"
#include "PmCertManager.hpp"

class PmPlatformConfiguration : public IPmPlatformConfiguration, public proxy::IProxyObserver
{
public:
    explicit PmPlatformConfiguration(std::shared_ptr<CMIDAPIProxyAbstract> cmidapi,
                                     std::shared_ptr<PackageManager::PmCertManager> certmgr);

    /**
     * @brief Retrieves the clients identity token. This token is used to identifcation/authentication when
     *   communicating with the cloud. This may return a cached value
     */
    bool GetIdentityToken(std::string &token) override;

    /**
     * @brief Retrieves the clients identity token. This token is used to identifcation/authentication when
     *   communicating with the cloud. This may return a cached value
     */
    bool GetUcIdentity(std::string &identity) override;

    /**
     * @brief Refreshes the UCID and UCID token values
     */
    bool RefreshIdentity() override;

    /**
     * @brief (Optional) Reloads ssl certs from the cert store
     *   Needed in Windows since curl can't load system certs without schannel
     */
    int32_t ReloadSslCertificates() override;

    /**
     * @brief (Optional) Retrieves the clients system certs
     *   Needed in Windows since curl can't load system certs without schannel
     *
     *  @param[in|out] certificates - Array of certs returned. The platfrom should allocated these
     *  @param[out] count - Number to certs returned
     */
    int32_t GetSslCertificates(X509 ***certificates, size_t &count) override;

    /**
     * @brief (Optional) Frees the cert list allocated by GetSslCertificates
     *
     *  @param[in] certificates - The cert array to be freed
     *  @param[in] count - Number to certs in the array
     */
    void ReleaseSslCertificates(X509 **certificates, size_t count) override;

    /**
     * @brief Provides the user agent for http requests
     */
    std::string GetHttpUserAgent() override;

    /**
     * @brief Gets the install directory
     */
    std::string GetInstallDirectory() override;

    /**
     * @brief Gets the log directory
     */
    std::string GetLogDirectory() const;

    /**
     * @brief Gets the data directory
     */
    std::string GetDataDirectory() override;

    /**
     * @brief Gets the PM version string
     */
    std::string GetPmVersion() override;

    /**
     * @brief Retrieve the PM URLs from the identity module
     */
    bool GetPmUrls(PmUrlList &urls) override;

    /**
     * @brief Retrieve the PM platform name, accepted by backend
     */
    virtual std::string GetPmPlatform() override;

    /**
     * @brief Retrieve the PM architecture name, accepted by backend
     */
    virtual std::string GetPmArchitecture() override;
    
    /**
     * @brief (Optional) On windows this triggers the Windows AIA mechanism to
     *   build out the certificate chain for the given URL
     *
     *  @param[in] url
     */
    bool UpdateCertStoreForUrl(const std::string &url) override;

    /**
     * @brief Starts the proxy discovery process
     *
     * @param[in] testUrl - url used to lookup a proxy using when discoving a WPAD proxy
     * @param[in] pacUrl - url of the PAC file. Used for WPAD proxies
     *
     * @return list of proxies discovered
     */
    std::list<PmProxy> StartProxyDiscovery(const std::string &testUrl, const std::string &pacUrl) override;

    /**
     * @brief Starts the async proxy discovery process
     *
     * @param[in] testUrl - url used to lookup a proxy using when discoving a WPAD proxy
     * @param[in] pacUrl - url of the PAC file. Used for WPAD proxies
     * @param[in] cb - callback to invoke when the process discovery is finished
     * @param[in] context - the context to provied to the callback
     *
     * @return true if the discovery process was started
     */
    bool StartProxyDiscoveryAsync(const std::string &testUrl, const std::string &pacUrl, AsyncProxyDiscoveryCb cb, void *context) override;

    /**
     * @brief callback that is called by pProxyEngine_ when discovery process is completed.
     *
     * @param[in] proxies - list of the discovered proxies
     *
     * @return void
     */
    void updateProxyList(const std::list<proxy::ProxyRecord>& proxies, const std::string& guid) override;

protected:
    cmid_result_t GetUrl( cmid_url_type_t urlType, std::string& url );

private:
    std::shared_ptr<CMIDAPIProxyAbstract> cmidapi_;
    std::shared_ptr<proxy::IProxyDiscoveryEngine> pProxyEngine_;
    std::unordered_map<std::string, std::pair<void*, AsyncProxyDiscoveryCb>> proxyCallbacks_;
    std::shared_ptr<PackageManager::PmCertManager> certmgr_;
};
