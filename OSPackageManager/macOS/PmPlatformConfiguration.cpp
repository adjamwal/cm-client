/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "PmPlatformConfiguration.hpp"
#include <string>

namespace
{
#if defined(PackageManager_VERSION_MAJOR) && defined(PackageManager_VERSION_MINOR)
const std::string kPmVersion = std::string(PackageManager_VERSION_MAJOR) + std::string(".") + std::string(PackageManager_VERSION_MINOR);
#else
constexpr std::string_view kPmVersion{"0.0"};
#endif

#ifdef CM_CONFIG_PATH
constexpr std::string_view kCmConfigPath{CM_CONFIG_PATH};
#else
constexpr std::string_view kCmConfigPath{"/opt/cisco/secureclient/cloudmanagement/etc"};
#endif

#ifdef CMID_DAEMON_PATH
constexpr std::string_view kCmidDaemonPath{CMID_DAEMON_PATH};
#else
constexpr std::string_view kCmidDaemonPath{"/opt/cisco/secureclient/cloudmanagement/bin"};
#endif

#ifdef CM_SHARED_LOG_PATH
constexpr std::string_view kCmSharedLogPath{CM_SHARED_LOG_PATH};
#elif __APPLE__
constexpr std::string_view kCmSharedLogPath{"/Library/Logs/Cisco/SecureClient/CloudManagement/"};
#else
constexpr std::string_view kCmSharedLogPath{"/var/logs/cisco/secureclient/cloudmanagement/"};
#endif

constexpr std::string_view kHttpUserAgentPrefix{"PackageManager/"};
}

PmPlatformConfiguration::PmPlatformConfiguration(std::shared_ptr<CMIDAPIProxyAbstract> cmidapi,
                                                 std::shared_ptr<PackageManager::PmCertManager> certmgr)
    :   cmidapi_(cmidapi),
        certmgr_(certmgr)
{
    certmgr_->LoadSystemSslCertificates();
}

bool PmPlatformConfiguration::GetIdentityToken(std::string& token)
{
    assert(cmidapi_);
    
    int buflen = 0;
    cmid_result_t result = cmidapi_->get_token(nullptr, &buflen);
    if (result != CMID_RES_INSUFFICIENT_LEN) {
        return false;
    }

    std::string cmid(buflen-1, '\0');
    result = cmidapi_->get_token(cmid.data(), &buflen);
    if (result != CMID_RES_SUCCESS) {
        return false;
    }

    token = cmid;
    return true;
}

bool PmPlatformConfiguration::GetUcIdentity(std::string& identity)
{
    assert(cmidapi_);

    int buflen = 0;
    cmid_result_t result = cmidapi_->get_id(nullptr, &buflen);
    if (result != CMID_RES_INSUFFICIENT_LEN) {
        return false;
    }
    
    std::string cmid(buflen-1, '\0');
    result = cmidapi_->get_id(cmid.data(), &buflen);
    if (result != CMID_RES_SUCCESS) {
        return false;
    }
    
    identity = cmid;
    return true;
}

bool PmPlatformConfiguration::RefreshIdentity()
{
    assert(cmidapi_);

    cmid_result_t result = cmidapi_->refresh_token();
    if (result != CMID_RES_SUCCESS) {
        return false;
    }
    return true;
}

int32_t PmPlatformConfiguration::ReloadSslCertificates()
{
    int32_t result = certmgr_->FreeSystemSslCertificates() ? 0 : -1;
    if (0 != result) {
        //TODO: LOG_ERROR
        return false;
    }
    
    result = certmgr_->LoadSystemSslCertificates() ? 0 : -1;
    if (0 != result) {
        //TODO: LOG_ERROR
        return false;
    }

    return result;
}

int32_t PmPlatformConfiguration::GetSslCertificates(X509 ***certificates, size_t &count)
{
    int32_t result = certmgr_->GetSslCertificates(certificates, count);
    return result;
}

void PmPlatformConfiguration::ReleaseSslCertificates(X509 **certificates, size_t count)
{
    certmgr_->ReleaseSslCertificates(certificates, count);
}

std::string PmPlatformConfiguration::GetHttpUserAgent()
{
    static std::string httpUserAgent = static_cast<std::string>(kHttpUserAgentPrefix) + static_cast<std::string>(GetPmVersion());
    return httpUserAgent;
}

std::string PmPlatformConfiguration::GetInstallDirectory()
{
    return static_cast<std::string>(kCmidDaemonPath);
}

std::string PmPlatformConfiguration::GetLogDirectory()
{
    return static_cast<std::string>(kCmSharedLogPath);
}

std::string PmPlatformConfiguration::GetDataDirectory()
{
    return  static_cast<std::string>(kCmConfigPath);
}

std::string PmPlatformConfiguration::GetPmVersion()
{
    return static_cast<std::string>(kPmVersion);
}

cmid_result_t PmPlatformConfiguration::GetUrl( cmid_url_type_t urlType, std::string& url )
{
    cmid_result_t result = CMID_RES_GENERAL_ERROR;
    int urlSize = 0;
    result = cmidapi_->get_url( urlType, nullptr, &urlSize );
    if( result == CMID_RES_INSUFFICIENT_LEN ) {
        std::string tmpUrl(urlSize-1, '\0');
        result = cmidapi_->get_url( urlType, tmpUrl.data(), &urlSize );
        if( result == CMID_RES_SUCCESS ) {
            url = tmpUrl;
        }
    }
    
    return result;
}

bool PmPlatformConfiguration::GetPmUrls(PmUrlList& urls)
{
    cmid_result_t rtn = CMID_RES_SUCCESS;
    cmid_result_t tmpRtn = CMID_RES_SUCCESS;
    
    tmpRtn = GetUrl( CMID_EVENT_URL, urls.eventUrl );
    if( tmpRtn != CMID_RES_SUCCESS ) {
//  TODO: oskryp, add error logging
//        LOG_ERROR( "Failed to fetch event url %d", tmpRtn );
        rtn = CMID_RES_GENERAL_ERROR;
    }
    
    tmpRtn = GetUrl( CMID_CHECKIN_URL, urls.checkinUrl );
    if( tmpRtn != CMID_RES_SUCCESS ) {
//  TODO: oskryp, add error logging
//        LOG_ERROR( "Failed to fetch checking url %d", tmpRtn );
        rtn = CMID_RES_GENERAL_ERROR;
    }
    
    tmpRtn = GetUrl( CMID_CATALOG_URL, urls.catalogUrl );
    if( tmpRtn != CMID_RES_SUCCESS ) {
//  TODO: oskryp, add error logging
//        LOG_ERROR( "Failed to fetch catalog url %d", tmpRtn );
        rtn = CMID_RES_GENERAL_ERROR;
    }
    
//  TODO: oskryp, add error logging
//    LOG_DEBUG( "Event Url %s Checkin Url %s Catalog Url %s", urls.eventUrl.c_str(), urls.checkinUrl.c_str(), urls.catalogUrl.c_str() );
    
    return rtn == CMID_RES_SUCCESS;
}

bool PmPlatformConfiguration::UpdateCertStoreForUrl(const std::string &url)
{
    (void) url;
    return false;
}

std::list<PmProxy> PmPlatformConfiguration::StartProxyDiscovery(const std::string &testUrl, const std::string &pacUrl)
{
    (void) testUrl;
    (void) pacUrl;
    return std::list<PmProxy>();
}

bool PmPlatformConfiguration::StartProxyDiscoveryAsync(const std::string &testUrl, const std::string &pacUrl, AsyncProxyDiscoveryCb cb, void *context)
{
    (void) testUrl;
    (void) pacUrl;
    (void) cb;
    (void) context;
    return false;
}
