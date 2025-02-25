/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include <string>
#include <cassert>
#include <sys/utsname.h>
#include "PmPlatformConfiguration.hpp"
#include "PmLogger.hpp"
#include "GuidUtil.hpp"

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
constexpr std::string_view kCmConfigPath{"/opt/cisco/secureclient/cloudmanagement/etc/"};
#endif

#ifdef CMID_DAEMON_PATH
constexpr std::string_view kCmidDaemonPath{CMID_DAEMON_PATH};
#else
constexpr std::string_view kCmidDaemonPath{"/opt/cisco/secureclient/cloudmanagement/bin"};
#endif

#ifdef CM_SHARED_LOG_PATH
constexpr std::string_view kCmSharedLogPath{CM_SHARED_LOG_PATH};
#else
constexpr std::string_view kCmSharedLogPath{"/var/logs/cisco/secureclient/cloudmanagement/"};
#endif

constexpr std::string_view kHttpUserAgentPrefix{"PackageManager/"};
constexpr std::string_view kHttpProxyType{"http"};
constexpr std::string_view kHttpsProxyType{"https"};

const std::string linux_platform{"linux"}; 
const std::string amd64{"amd64"};
const std::string arm64{"arm64"};

auto determineArch = [](){
    std::string sPath;
    struct utsname sysinfo{};
    uname(&sysinfo);
    sPath =  std::string(sysinfo.machine) == "aarch64"
                                        ? arm64
                                        : amd64;
    
    return sPath;
};

static std::string sArchForConfig = determineArch();

}

PmPlatformConfiguration::PmPlatformConfiguration(std::shared_ptr<CMIDAPIProxyAbstract> cmidapi)
    :   cmidapi_(std::move(cmidapi))
{
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
    return 0;
}

int32_t PmPlatformConfiguration::GetSslCertificates(X509 ***certificates, size_t &count)
{
    (void) certificates;
    (void) count;
    return 0;
}

void PmPlatformConfiguration::ReleaseSslCertificates(X509 **certificates, size_t count)
{
    (void) certificates;
    (void) count;
}

std::string PmPlatformConfiguration::GetHttpUserAgent()
{
    static const std::string httpUserAgent = static_cast<std::string>(kHttpUserAgentPrefix) + static_cast<std::string>(GetPmVersion());
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

#if defined(DEBUG) && LOCAL_WEBSERVER_OVERRIDE
    switch ( urlType ) {
        case CMID_CHECKIN_URL:
            url = "http://localhost:5000/checkin";
            return CMID_RES_SUCCESS;

        case CMID_CATALOG_URL:
            url = "http://localhost:5000/catalog";
            return CMID_RES_SUCCESS;

        case CMID_EVENT_URL:
        default:
            [[fallthrough]];
    }
#endif // LOCAL_WEBSERVER_OVERRIDE
    assert(cmidapi_);
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
        PM_LOG_ERROR("Failed to fetch event url %d", tmpRtn);
        rtn = CMID_RES_GENERAL_ERROR;
    }
    
    tmpRtn = GetUrl( CMID_CHECKIN_URL, urls.checkinUrl );
    if( tmpRtn != CMID_RES_SUCCESS ) {
        PM_LOG_ERROR("Failed to fetch checking url %d", tmpRtn);
        rtn = CMID_RES_GENERAL_ERROR;
    }
    
    tmpRtn = GetUrl( CMID_CATALOG_URL, urls.catalogUrl );
    if( tmpRtn != CMID_RES_SUCCESS ) {
        PM_LOG_ERROR("Failed to fetch catalog url %d", tmpRtn);
        rtn = CMID_RES_GENERAL_ERROR;
    }
    
    PM_LOG_DEBUG("Event Url %s Checkin Url %s Catalog Url %s", urls.eventUrl.c_str(), urls.checkinUrl.c_str(), urls.catalogUrl.c_str());

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
    return true;
}

std::string PmPlatformConfiguration::GetPmPlatform()
{
    return linux_platform;
}

std::string PmPlatformConfiguration::GetPmArchitecture()
{
    return sArchForConfig;
}

