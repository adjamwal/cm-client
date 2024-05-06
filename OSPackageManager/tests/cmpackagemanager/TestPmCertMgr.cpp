#include "gtest/gtest.h"

#include "OSPackageManager/util/ScopedGuard.hpp"
#include "MockPmCertManager/MockPmCertRetriever.h"
#include "PmCertManager.hpp"
#include "UnitTestBase.h"
#include "PmLogger.hpp"

#include <memory>
#include <CoreFoundation/CFString.h>
#include <Security/Security.h>

using testing::StrictMock;
using testing::Return;
using ::testing::SetArgReferee;
using ::testing::ElementsAreArray;
using testing::_;

class TestPmCertMgr: public TestEnv::UnitTestBase{
public:
    TestPmCertMgr()
    {
        m_patient.reset(new PackageManager::PmCertManager(mockEnv_.certRetriever_));
    }
protected:
    std::unique_ptr<PackageManager::PmCertManager> m_patient;
};

TEST_F(TestPmCertMgr, LoadWillFree)
{
    EXPECT_CALL(*mockEnv_.certRetriever_, LoadSystemSslCertificates()).Times(1);
    EXPECT_CALL(*mockEnv_.certRetriever_, FreeSystemSslCertificates()).Times(1);

    ON_CALL( *mockEnv_.certRetriever_, LoadSystemSslCertificates() ).WillByDefault( Return( true ) );
    m_patient->LoadSystemSslCertificates();
}

TEST_F(TestPmCertMgr, FreeWontLoad)
{
    EXPECT_CALL(*mockEnv_.certRetriever_, LoadSystemSslCertificates()).Times(0);
    //On invoke + on destruction
    EXPECT_CALL(*mockEnv_.certRetriever_, FreeSystemSslCertificates()).Times(2);

    ON_CALL( *mockEnv_.certRetriever_, FreeSystemSslCertificates() ).WillByDefault( Return( true ) );
    m_patient->FreeSystemSslCertificates();
}

const std::string sCert1{"MIIDxTCCAq2gAwIBAgIBADANBgkqhkiG9w0BAQsFADCBgzELMAkGA1UEBhMCVVMxEDAOBgNVBAgTB0FyaXpvbmExEzARBgNVBAcTClNjb3R0c2RhbGUxGjAYBgNVBAoTEUdvRGFkZHkuY29tLCBJbmMuMTEwLwYDVQQDEyhHbyBEYWRkeSBSb290IENlcnRpZmljYXRlIEF1dGhvcml0eSAtIEcyMB4XDTA5MDkwMTAwMDAwMFoXDTM3MTIzMTIzNTk1OVowgYMxCzAJBgNVBAYTAlVTMRAwDgYDVQQIEwdBcml6b25hMRMwEQYDVQQHEwpTY290dHNkYWxlMRowGAYDVQQKExFHb0RhZGR5LmNvbSwgSW5jLjExMC8GA1UEAxMoR28gRGFkZHkgUm9vdCBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkgLSBHMjCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAL9xYgjx+lk09xvJGKP3gElY6SKDE6bFIEMBO4Tx5oVJnyfq9oQbTqC023CYxzIBsQU+B07u9PpPL1kwIuerGVZr4oAH/PMWdYA5UXvl+TW2dE6pjYIT5LY/qQOD+qK+ihVqf94Lw7YZFAXK6sOoBJQ7RnwyDfMAZiLIjWltNowRGLfTshxgtDj6AozO091GB94KPutdfMh8+7ArU6SSYmlRJQVhGkSBjCypQ5Yj36w6gZoOKcUcqeldHraenjAKOc7xiID7S13MMuyFYkMlNAJWJwGRtDtwKj9useiciAF9n9T521NtYJ2/LOdYq7hfRvzOxBsDPAnrSTFcaUaz4EcCAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAQYwHQYDVR0OBBYEFDqahQcQZyi27/a9BUFuIMGU2g/eMA0GCSqGSIb3DQEBCwUAA4IBAQCZ21151fmXWWcDYfF+OwYxdS2hII5PZYe096acvNjpL9DbWu7PdIxztDhC2gV7+AJ1uP2lsdeu9tfeE8tTEH6KRtGX+rcuKxGrkLAngPnon1rpN5+r5N9ss4UXnT3ZJE95kTXWXwTrgIOrmgIttRD02JDHBHNA7XIloKmf7J6raBKZV8aPEjoJpL1E/QYVN8Gb5DKj7Tjo2GTzLH4U/ALqn83/B2gX2yKQOC16jdFU8WnjXzPKej17CuPKf1855eJ1usV2GDPOLPAvTK33sefOT6jEm0pUBsV/fdUID+Ic/n4XuKxe9tQWskMJDE32p2u0mYRlynqI4uJEvlz36hz1"};
const std::string sCert2{"MIICVDCCAdugAwIBAgIQZ3SdjXfYO2rbIvT/WeK/zjAKBggqhkjOPQQDAzBsMQswCQYDVQQGEwJHUjE3MDUGA1UECgwuSGVsbGVuaWMgQWNhZGVtaWMgYW5kIFJlc2VhcmNoIEluc3RpdHV0aW9ucyBDQTEkMCIGA1UEAwwbSEFSSUNBIFRMUyBFQ0MgUm9vdCBDQSAyMDIxMB4XDTIxMDIxOTExMDExMFoXDTQ1MDIxMzExMDEwOVowbDELMAkGA1UEBhMCR1IxNzA1BgNVBAoMLkhlbGxlbmljIEFjYWRlbWljIGFuZCBSZXNlYXJjaCBJbnN0aXR1dGlvbnMgQ0ExJDAiBgNVBAMMG0hBUklDQSBUTFMgRUNDIFJvb3QgQ0EgMjAyMTB2MBAGByqGSM49AgEGBSuBBAAiA2IABDgI/rGgltJ6rK9JOtDA4MM7KKrxcm1lAEeIhPyaJmuqS7psBAqIXhfyVYf8MLA04jRYVxqEU+kw2anylnTDUR9YSTHMmE5gEYd103KUkE+bECUqqHgtvpBBWJAVcqeht6NCMEAwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUyRtTgRL+BNUW0aq8mm+3oJUZbsowDgYDVR0PAQH/BAQDAgGGMAoGCCqGSM49BAMDA2cAMGQCMBHervjcToiwqfAircJRQO9gcS3ujwLEXQNwSaSS6sUUiHCm0w2wqsosQJz76YJumgIwK0eaB8bRwoF8yguWGEEbo/QwCZ61IygNnxS2PFOiTAZpffpskcYqSUXm7LcT4Tps"};

using certificatesBase64V = std::vector<std::string>;
const std::vector<certificatesBase64V> vCerts{
    {sCert1},
    {sCert1, sCert2}
};

class TestPmCertMgrPar : public ::testing::TestWithParam<certificatesBase64V>
{
public:
    TestPmCertMgrPar()
        : _retrieverMock(std::make_shared<::testing::StrictMock<MockPmCertRetriever>>()),
        m_patient(std::make_unique<PackageManager::PmCertManager>(_retrieverMock)),
        _certificates(GetParam())
    {
        x509certificates_.reserve(_certificates.size());
        for (auto &certBase64: _certificates)
        {
            auto certEnc64S = CFStringCreateWithCString(kCFAllocatorDefault, certBase64.c_str(), kCFStringEncodingASCII);
            CFDataRef dataToDecode = CFStringCreateExternalRepresentation(kCFAllocatorDefault, certEnc64S, kCFStringEncodingASCII, 0);
            SecTransformRef decodeTransform = SecDecodeTransformCreate(kSecBase64Encoding, nullptr);
            SecTransformSetAttribute(decodeTransform, kSecTransformInputAttributeName,
                                     dataToDecode, nullptr);
            CFDataRef decodedData = (CFDataRef)SecTransformExecute(decodeTransform, nullptr);
            auto cert = SecCertificateCreateWithData(kCFAllocatorDefault, decodedData);
            auto certData = SecCertificateCopyData((SecCertificateRef)cert);
            auto certDataPtr = CFDataGetBytePtr(certData);
            X509* x509Cert = X509_dup(d2i_X509( NULL, ( const unsigned char** )&certDataPtr, CFDataGetLength(certData) ));
            x509certificates_.push_back(x509Cert);
            CFRelease(certData);
            CFRelease(cert);
            CFRelease(decodedData);
            CFRelease(decodeTransform);
            CFRelease(dataToDecode);
            CFRelease(certEnc64S);
        }

        ON_CALL(*_retrieverMock, GetSslCertificates(_) ).WillByDefault( Return( (int32_t)x509certificates_.size() ));
        ON_CALL(*_retrieverMock, GetSslCertificates(_)).WillByDefault(SetArgReferee<0>(x509certificates_));
        PmLogger::initLogger();
    }
    
    ~TestPmCertMgrPar()
    {
        for (auto cert: x509certificates_ ) {
            if ( cert )
                X509_free(cert);
        }
        PmLogger::releaseLogger();
    }
protected:
    certificatesBase64V _certificates;
    std::vector<X509*> x509certificates_;
    std::shared_ptr<::testing::StrictMock<MockPmCertRetriever>> _retrieverMock;
    std::unique_ptr<PackageManager::PmCertManager> m_patient;
};


INSTANTIATE_TEST_SUITE_P(Certs,
                         TestPmCertMgrPar, ::testing::ValuesIn(vCerts));

TEST_P(TestPmCertMgrPar, CertificatesAreSame)
{
    EXPECT_CALL(*_retrieverMock, GetSslCertificates(_)).Times(1);
    EXPECT_CALL(*_retrieverMock, FreeSystemSslCertificates()).Times(1);
    X509** certificates = nullptr;
    size_t nCount = 0;
    auto guard = util::scoped_guard([=]() {
        for( size_t i = 0; i < nCount; ++i ) {
            X509_free( certificates[ i ] );
            certificates[ i ] = nullptr;
        }
    });
    
    certificatesBase64V vCertsBase64;
    ASSERT_EQ(m_patient->GetSslCertificates(&certificates, nCount), 0);
    vCertsBase64.reserve(nCount);
    for( size_t i = 0; i < nCount; ++i ) {
        CFIndex dsize = (CFIndex)i2d_X509(certificates[i], NULL);
        unsigned char* x509Data = new unsigned char[dsize];
        unsigned char* px509Data = x509Data;
        dsize = (CFIndex)i2d_X509(certificates[i], &px509Data);
        auto data = CFDataCreate(kCFAllocatorDefault, (const UInt8*)x509Data, dsize);
        SecTransformRef encoder = SecEncodeTransformCreate(kSecBase64Encoding, nullptr);
        SecTransformSetAttribute(encoder, kSecTransformInputAttributeName, data, nullptr);
        CFDataRef encodedData = (CFDataRef)SecTransformExecute(encoder, nullptr);
        
        CFStringRef encString = CFStringCreateFromExternalRepresentation(kCFAllocatorDefault, encodedData, kCFStringEncodingASCII);
        vCertsBase64.push_back(CFStringGetCStringPtr(encString, kCFStringEncodingASCII));
        
        delete []x509Data;
        CFRelease(encString);
        CFRelease(encodedData);
        CFRelease(encoder);
        CFRelease(data);
    }
    EXPECT_THAT(vCertsBase64, ElementsAreArray(_certificates));
    
    EXPECT_EQ(nCount, x509certificates_.size());
}
