#include "PmCodesignVerifier.hpp"
#include "PmLogger.hpp"
#include <gtest/gtest.h>

namespace {  /* anonymous namespace */
    
    std::string GetSignedBinaryPath() {
        return {"./signedprocess"};
    }

    std::string GetUnsignedBinaryPath() {
        return {"./PackageManagerUnitTest"};
    }

} /* end anonymous namespace */

TEST(CodesignVerifier, Verify)
{
    PmLogger::initLogger();
    CodesignVerifier codesign;

    EXPECT_EQ(CodeSignStatus::CODE_SIGN_OK, codesign.Verify(GetSignedBinaryPath(),
                                                    SIGNER_CISCO,
                                                    SIGTYPE_DEFAULT)
                                                    );

    // Check if CODE_SIGN_FAIL is returned when empty file path
    // and empty signer is passed as an argument to Verify
    EXPECT_EQ(CodeSignStatus::CODE_SIGN_FAIL, codesign.Verify("",
                                                        "",
                                                        SIGTYPE_DEFAULT)
                                                        );

    // Check if CODE_SIGN_FAIL is returned when signer name other than
    // Sig_Type::SIGTPE_NATIVE is passed as an argument to Verify
    EXPECT_EQ(CodeSignStatus::CODE_SIGN_FAIL, codesign.Verify(GetSignedBinaryPath(),
                                                        SIGNER_CISCO,
                                                        SigType::SIGTYPE_AUTO)
                                                        );

    // Check if CODE_SIGN_VERIFICATION_FAILED is returned when
    // sign verify fails
    EXPECT_EQ(CodeSignStatus::CODE_SIGN_VERIFICATION_FAILED, codesign.Verify(GetUnsignedBinaryPath(),
                                                        SIGNER_CISCO,
                                                        SIGTYPE_DEFAULT)
                                                        );
    PmLogger::releaseLogger();
}
