#pragma once

#include "gmock/gmock.h"

#include "IPmCertRetriever.hpp"
#include "PmCertManager.hpp"

class MockPmCertManager: public PackageManager::PmCertManager{
public:
    explicit MockPmCertManager(std::shared_ptr<PackageManager::IPmCertRetriever> certRetriever)
        :PackageManager::PmCertManager(certRetriever){}
    ~MockPmCertManager() = default;
    
    MOCK_METHOD(bool, LoadSystemSslCertificates, (), (override));
    MOCK_METHOD(bool, FreeSystemSslCertificates, (), (override));
    MOCK_METHOD(int32_t, GetSslCertificates, (X509 ***, size_t &), (override));
    MOCK_METHOD(void, ReleaseSslCertificates, (X509 **, size_t), (override));
};

