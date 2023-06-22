/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */
#pragma once

#include "IPmCertRetriever.hpp"
#include <vector>

namespace PackageManager
{
class PmCertRetrieverImpl : public IPmCertRetriever{
public:
    PmCertRetrieverImpl() = default;
    virtual ~PmCertRetrieverImpl();
    
    virtual bool LoadSystemSslCertificates() override;
    virtual bool FreeSystemSslCertificates() override;
    
    /**
     * @brief (Optional) Retrieves the clients system certs
     *
     *  @param[out] certificates - Vector of certs returned. The platfrom should allocated these
     */
    virtual int32_t GetSslCertificates(std::vector<X509*> &certificates) override;

protected:
    std::vector<X509*> certificates_;

};

}
