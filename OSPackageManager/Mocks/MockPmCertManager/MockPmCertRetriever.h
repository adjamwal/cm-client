#pragma once

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "IPmCertRetriever.hpp"


class MockPmCertRetriever: public PackageManager::IPmCertRetriever{
public:
    MockPmCertRetriever();
    ~MockPmCertRetriever();
    
    MOCK_METHOD(bool, LoadSystemSslCertificates, (), (override));
    MOCK_METHOD(bool, FreeSystemSslCertificates, (), (override));
    MOCK_METHOD(int32_t, GetSslCertificates, (std::vector<X509*>&), (override));
};
