/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */
#pragma once

#include "IPmCertRetriever.hpp"
#include <openssl/ssl.h>
#include <memory>

namespace PackageManager
{
class PmCertManager{
public:
    explicit PmCertManager(std::shared_ptr<IPmCertRetriever> certRetriever);
    virtual ~PmCertManager();
    
    virtual bool LoadSystemSslCertificates();
    virtual bool FreeSystemSslCertificates();
    
    /**
     * @brief (Optional) Retrieves the clients system certs
     *
     *  @param[in|out] certificates - Array of certs returned. The platfrom should allocated these
     *  @param[out] count - Number to certs returned
     */
    virtual int32_t GetSslCertificates(X509 ***certificates, size_t &count);

    /**
     * @brief (Optional) Frees the cert list allocated by GetSslCertificates
     *
     *  @param[in] certificates - The cert array to be freed
     *  @param[in] count - Number to certs in the array
     */
    virtual void ReleaseSslCertificates(X509 **certificates, size_t count);

protected:
    std::shared_ptr<IPmCertRetriever> certRetriever_;
};

}
