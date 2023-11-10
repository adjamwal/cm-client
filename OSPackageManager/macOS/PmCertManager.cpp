/**
 * @file
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#include "PmCertManager.hpp"
#include "PmLogger.hpp"

#include <CoreFoundation/CFArray.h>
#include <Security/Security.h>
#include <string>
#include <vector>

namespace PackageManager
{
PmCertManager::PmCertManager(std::shared_ptr<IPmCertRetriever> certRetriever)
    :certRetriever_(std::move(certRetriever)){}

bool PmCertManager::LoadSystemSslCertificates()
{
    bool bRet = false;
    if ( certRetriever_ ){
        bRet = certRetriever_->LoadSystemSslCertificates();
    }
    
    return bRet;
}
bool PmCertManager::FreeSystemSslCertificates()
{
    bool bRet = false;
    if ( certRetriever_ ){
        bRet = certRetriever_->FreeSystemSslCertificates();
    }

    return bRet;
}

int32_t PmCertManager::GetSslCertificates( X509*** certificates, size_t& count )
{
    int32_t result = -1;
    count = 0;

    std::vector<X509*> certVector;
    if (certRetriever_)
        certRetriever_->GetSslCertificates(certVector);

    if (nullptr != certificates && !certVector.empty() ){
        *certificates = ( X509** )calloc( certVector.size(), sizeof(decltype(certVector)::value_type) );

        for (auto cert: certVector) {
            ( *certificates )[ count ] = cert;
            ( count )++;
        }

        result = 0;
    }

    PM_LOG_INFO("Returning %d certificates", count);

    return result;
}

void PmCertManager::ReleaseSslCertificates(X509 **certificates, size_t count)
{
    if( !certificates )
        return;
    
    for( size_t i = 0; i < count; ++i ) {
        X509_free( certificates[ i ] );
        certificates[ i ] = nullptr;
    }

    free( certificates );
    certificates = nullptr;

    PM_LOG_INFO("Released %d certificates", count);
}

PmCertManager::~PmCertManager()
{
    FreeSystemSslCertificates();
}

}
