#include "gtest/gtest.h"

#include "MockPmCertManager/MockPmCertRetriever.h"
#include "PmCertManager.hpp"
#include "UnitTestBase.h"

#include <memory>

using testing::StrictMock;
using testing::Return;
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


