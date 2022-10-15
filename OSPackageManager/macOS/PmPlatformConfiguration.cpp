/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "PmPlatformConfiguration.hpp"

bool GetIdentityToken(std::string& token)
{
    (void) token;
    return false;
}

bool GetUcIdentity(std::string& identity)
{
    (void) identity;
    return false;
}

bool RefreshIdentity()
{
    return false;
}

int32_t ReloadSslCertificates()
{
    return -1;
}

int32_t GetSslCertificates(X509 ***certificates, size_t &count)
{
    (void) certificates;
    (void) count;
    return -1;
}

void ReleaseSslCertificates(X509 **certificates, size_t count)
{
    (void) certificates;
    (void) count;
}

std::string GetHttpUserAgent()
{
     return "";
}

std::string GetInstallDirectory()
{
     return "";
}

std::string GetDataDirectory()
{
     return "";
}

std::string GetPmVersion()
{
     return "";
}

bool GetPmUrls(PmUrlList& urls)
{
     (void) urls;
     return false;
}

bool UpdateCertStoreForUrl(const std::string &url)
{
    (void) url;
    return false;
}

std::list<PmProxy> StartProxyDiscovery(const std::string &testUrl, const std::string &pacUrl)
{
    (void) testUrl;
    (void) pacUrl;
    return std::list<PmProxy>();
}

bool StartProxyDiscoveryAsync(const std::string &testUrl, const std::string &pacUrl, AsyncProxyDiscoveryCb cb, void *context)
{
    (void) testUrl;
    (void) pacUrl;
    (void) cb;
    (void) context;
    return false;
}
