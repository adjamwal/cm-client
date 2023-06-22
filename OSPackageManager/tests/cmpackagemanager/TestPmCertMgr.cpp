#include "gtest/gtest.h"

#include "MockPmCertManager/MockPmCertRetriever.h"
#include "PmCertManager.hpp"

#include <memory>

using testing::StrictMock;
using testing::Return;
using testing::_;

class TestPmCertMgr: public ::testing::Test{
public:
    TestPmCertMgr()
    {
        certRetriever_ = std::make_shared<StrictMock<MockPmCertRetriever>>();
        m_patient.reset(new PackageManager::PmCertManager(certRetriever_));
    }
protected:
    std::shared_ptr<MockPmCertRetriever> certRetriever_;
    std::unique_ptr<PackageManager::PmCertManager> m_patient;
};

TEST_F(TestPmCertMgr, LoadWillFree)
{
    EXPECT_CALL(*certRetriever_, LoadSystemSslCertificates()).Times(1);
    EXPECT_CALL(*certRetriever_, FreeSystemSslCertificates()).Times(1);

    ON_CALL( *certRetriever_, LoadSystemSslCertificates() ).WillByDefault( Return( true ) );
    m_patient->LoadSystemSslCertificates();
}

TEST_F(TestPmCertMgr, FreeWontLoad)
{
    EXPECT_CALL(*certRetriever_, LoadSystemSslCertificates()).Times(0);
    //On invoke + on destruction
    EXPECT_CALL(*certRetriever_, FreeSystemSslCertificates()).Times(2);

    ON_CALL( *certRetriever_, FreeSystemSslCertificates() ).WillByDefault( Return( true ) );
    m_patient->FreeSystemSslCertificates();
}


