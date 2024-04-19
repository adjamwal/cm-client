/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#pragma once
#include "PackageManager/IPmPlatformConfiguration.h"
#include "gmock/gmock.h"

class MockPmPlatformConfiguration : public IPmPlatformConfiguration
{
  public:
    MOCK_METHOD(bool, GetIdentityToken, (std::string& token), (override));
    MOCK_METHOD(bool, GetUcIdentity, (std::string& identity), (override));
    MOCK_METHOD(bool, RefreshIdentity, (), (override));
    MOCK_METHOD(int32_t, ReloadSslCertificates, (), (override));
    MOCK_METHOD(int32_t, GetSslCertificates, (X509*** certificates, size_t &count), (override));
    MOCK_METHOD(void, ReleaseSslCertificates, (X509** certificates, size_t count), (override));
    MOCK_METHOD(std::string, GetHttpUserAgent, (), (override));
    MOCK_METHOD(std::string, GetInstallDirectory, (), (override));
    MOCK_METHOD(std::string, GetDataDirectory, (), (override));
    MOCK_METHOD(std::string, GetPmVersion, (), (override));
    MOCK_METHOD(bool, GetPmUrls, (PmUrlList& urls), (override));
    MOCK_METHOD(std::string, GetPmPlatform, (), (override));
    MOCK_METHOD(std::string, GetPmArchitecture, (), (override));
    MOCK_METHOD(bool, UpdateCertStoreForUrl, (const std::string& url), (override));
    MOCK_METHOD(std::list<PmProxy>, StartProxyDiscovery, (const std::string& testUrl, const std::string& pacUrl), (override));
    MOCK_METHOD(bool, StartProxyDiscoveryAsync, (const std::string& testUrl, const std::string& pacUrl, AsyncProxyDiscoveryCb cb, void* context), (override));
};
