/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "PackageManager/IPmPlatformConfiguration.h"
#include "../proxy/CMIDAPIProxyAbstract.hpp"

#include "cmid/CMIDAPI.h"
#include "PmCertManager.hpp"

class PmPlatformConfiguration : public IPmPlatformConfiguration
{
public:
    explicit PmPlatformConfiguration(std::shared_ptr<CMIDAPIProxyAbstract> cmidapi,
                                     std::shared_ptr<PackageManager::PmCertManager> certmgr);

    /**
     * @brief Retrieves the clients identity token. This token is used to identifcation/authentication when
     *   communicating with the cloud. This may return a cached value
     */
    bool GetIdentityToken(std::string &token);

    /**
     * @brief Retrieves the clients identity token. This token is used to identifcation/authentication when
     *   communicating with the cloud. This may return a cached value
     */
    bool GetUcIdentity(std::string &identity);

    /**
     * @brief Refreshes the UCID and UCID token values
     */
    bool RefreshIdentity();

    /**
     * @brief (Optional) Reloads ssl certs from the cert store
     *   Needed in Windows since curl can't load system certs without schannel
     */
    int32_t ReloadSslCertificates();

    /**
     * @brief (Optional) Retrieves the clients system certs
     *   Needed in Windows since curl can't load system certs without schannel
     *
     *  @param[in|out] certificates - Array of certs returned. The platfrom should allocated these
     *  @param[out] count - Number to certs returned
     */
    int32_t GetSslCertificates(X509 ***certificates, size_t &count);

    /**
     * @brief (Optional) Frees the cert list allocated by GetSslCertificates
     *
     *  @param[in] certificates - The cert array to be freed
     *  @param[in] count - Number to certs in the array
     */
    void ReleaseSslCertificates(X509 **certificates, size_t count);

    /**
     * @brief Provides the user agent for http requests
     */
    std::string GetHttpUserAgent();

    /**
     * @brief Gets the install directory
     */
    std::string GetInstallDirectory();

    /**
     * @brief Gets the data directory
     */
    std::string GetDataDirectory();

    /**
     * @brief Gets the PM version string
     */
    std::string GetPmVersion();

    /**
     * @brief Retrieve the PM URLs from the identity module
     */
    bool GetPmUrls(PmUrlList &urls);

    /**
     * @brief (Optional) On windows this triggers the Windows AIA mechanism to
     *   build out the certificate chain for the given URL
     *
     *  @param[in] url
     */
    bool UpdateCertStoreForUrl(const std::string &url);

    /**
     * @brief Starts the proxy discovery process
     *
     * @param[in] testUrl - url used to lookup a proxy using when discoving a WPAD proxy
     * @param[in] pacUrl - url of the PAC file. Used for WPAD proxies
     *
     * @return list of proxies discovered
     */
    std::list<PmProxy> StartProxyDiscovery(const std::string &testUrl, const std::string &pacUrl);

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
    bool StartProxyDiscoveryAsync(const std::string &testUrl, const std::string &pacUrl, AsyncProxyDiscoveryCb cb, void *context);
    
protected:
    cmid_result_t GetUrl( cmid_url_type_t urlType, std::string& url );

private:
    std::shared_ptr<CMIDAPIProxyAbstract> cmidapi_;
    std::shared_ptr<PackageManager::PmCertManager> certmgr_;
};
