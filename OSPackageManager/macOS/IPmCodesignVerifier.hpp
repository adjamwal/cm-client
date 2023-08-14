#pragma once
#include <string>
#include <filesystem>

#define SIGTYPE_DEFAULT     SigType::SIGTYPE_NATIVE

enum class CodeSignStatus {
    CODE_SIGN_OK = 0,
    CODE_SIGN_FAIL = -1,
    CODE_SIGN_EXPIRED = -14,  /* Codesign time expired */
    CODE_SIGNER_MISMATCH = -15, /* Untrusted signer disallowed */
    CODE_SIGN_VERIFICATION_FAILED = -16, /* Codesign verification failure */
};

enum class SigType
{
    SIGTYPE_AUTO = 0,
    SIGTYPE_NATIVE,
    SIGTYPE_CISCO
};

class IPmCodesignVerifier
{
public:
    virtual ~IPmCodesignVerifier() = default;
    virtual CodeSignStatus ExecutableVerify( const std::filesystem::path& path, const std::string& strSigner, SigType sig_type ) = 0;
    virtual CodeSignStatus ExecutableVerifyWithKilldate( const std::filesystem::path& path, const std::string& strSigner, SigType sig_type, uint64_t killdate ) = 0;
    virtual CodeSignStatus PackageVerify( const std::filesystem::path& path, const std::string& strSigner ) = 0;
};
