/**
 * @file
 *
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved
 */
#pragma once

#include <openssl/ssl.h>
#include <vector>

namespace PackageManager
{
class IPmCertRetriever{
public:
    IPmCertRetriever() = default;
    virtual ~IPmCertRetriever() = default;
    
    virtual bool LoadSystemSslCertificates() = 0;
    virtual bool FreeSystemSslCertificates() = 0;
    
    /**
     * @brief (Optional) Retrieves the clients system certs
     *
     *  @param[out] certificates - Vector of certs returned. The platfrom should allocate these.
     */
    virtual int32_t GetSslCertificates(std::vector<X509*> &certificates) = 0;
};

}
