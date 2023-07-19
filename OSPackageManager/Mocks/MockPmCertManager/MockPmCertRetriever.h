/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#pragma once

#include "IPmCertRetriever.hpp"
#include "gmock/gmock.h"

class MockPmCertRetriever: public PackageManager::IPmCertRetriever
{
public:
    MockPmCertRetriever();
    ~MockPmCertRetriever();
    
    MOCK_METHOD(bool, LoadSystemSslCertificates, (), (override));
    MOCK_METHOD(bool, FreeSystemSslCertificates, (), (override));
    MOCK_METHOD(int32_t, GetSslCertificates, (std::vector<X509*>&), (override));
};
