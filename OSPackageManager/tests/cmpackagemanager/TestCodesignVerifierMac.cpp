#include <gtest/gtest.h>

#include "PmCodesignVerifier.hpp"
#include "PmLogger.hpp"
#include "PmPkgUtilWrapper.hpp"

namespace {  /* anonymous namespace */
    
    std::string GetSignedBinaryPath() {
        return {"./signedprocess"};
    }

    std::string GetUnsignedBinaryPath() {
        return {"./PackageManagerUnitTest"};
    }

    std::string GetSignedPackagePath() {
        return {"./test_package.pkg"};
    }

} /* end anonymous namespace */

TEST(CodesignVerifier, Verify)
{
    PmLogger::initLogger();
    CodesignVerifier codesign(std::make_shared<PmPkgUtilWrapper>());

    EXPECT_EQ(CodeSignStatus::CODE_SIGN_OK, codesign.ExecutableVerify(GetSignedBinaryPath(),
                                                    SIGNER_CISCO_BIN,
                                                    SIGTYPE_DEFAULT)
                                                    );

    // Check if CODE_SIGN_FAIL is returned when empty file path
    // and empty signer is passed as an argument to Verify
    EXPECT_EQ(CodeSignStatus::CODE_SIGN_FAIL, codesign.ExecutableVerify("",
                                                        "",
                                                        SIGTYPE_DEFAULT)
                                                        );

    // Check if CODE_SIGN_FAIL is returned when signer name other than
    // Sig_Type::SIGTPE_NATIVE is passed as an argument to Verify
    EXPECT_EQ(CodeSignStatus::CODE_SIGN_FAIL,
              codesign.ExecutableVerify(GetSignedBinaryPath(),
                                                        SIGNER_CISCO_BIN,
                                                        SigType::SIGTYPE_AUTO)
                                                        );

    // Check if CODE_SIGN_VERIFICATION_FAILED is returned when
    // sign verify fails
    EXPECT_EQ(CodeSignStatus::CODE_SIGN_VERIFICATION_FAILED,
              codesign.ExecutableVerify(GetUnsignedBinaryPath(),
                                                        SIGNER_CISCO_BIN,
                                                        SIGTYPE_DEFAULT)
                                                        );

    EXPECT_EQ(CodeSignStatus::CODE_SIGN_OK, codesign.PackageVerify(
                                               GetSignedPackagePath(),
                                               SIGNER_CISCO_PKG
                                            )
              );

    PmLogger::releaseLogger();
}
