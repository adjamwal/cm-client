/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include <string>
#include "PmPlatformConfiguration.hpp"

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
    return -1;
}

int32_t PmPlatformConfiguration::GetSslCertificates(X509 ***certificates, size_t &count)
{
    (void) certificates;
    (void) count;
    return -1;
}

void PmPlatformConfiguration::ReleaseSslCertificates(X509 **certificates, size_t count)
{
    (void) certificates;
    (void) count;
}

std::string PmPlatformConfiguration::GetHttpUserAgent()
{
     return "";
}

std::string PmPlatformConfiguration::GetInstallDirectory()
{
     return "";
}

std::string PmPlatformConfiguration::GetDataDirectory()
{
     return "";
}

std::string PmPlatformConfiguration::GetPmVersion()
{
     return "";
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
