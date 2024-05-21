/**************************************************************************
 * Copyright (c) 2022, Cisco Systems, All Rights Reserved
 ***************************************************************************
 *
 * @file : PmCodesignVerifierMac.cpp
 *
 ***************************************************************************
 *
 * Implements file signature verification
 *
 ***************************************************************************/
#include "util/codesign_pkg.h"
#include "util/codesign_bin.h"
#include "PmCodesignVerifier.hpp"
#include "PmLogger.hpp"
#include "IPmPkgUtil.hpp"

CodesignVerifier::CodesignVerifier(std::shared_ptr<IPmPkgUtil> pkgUtil) : pkgUtil_(std::move(pkgUtil))
{

}

CodeSignStatus CodesignVerifier::ExecutableVerifyWithKilldate( const std::filesystem::path& path, const std::string& strSigner, SigType sig_type, uint64_t killdate )
{
    CodeSignStatus status = CodeSignStatus::CODE_SIGN_VERIFICATION_FAILED;
    if (path.empty() || strSigner.empty())
    {
        PM_LOG_DEBUG("Invalid Input" );
        return status;
    }
    PM_LOG_DEBUG("verifying file signature: file = [%s], signer = [%s], type = [%d]", path.string().c_str(), strSigner.c_str(), sig_type );
    int iRet = check_signature(path.string().c_str());
    if (iRet >= 0)
    {
        iRet = check_signer(path.string().c_str(), strSigner.c_str(), sig_type, killdate);
        if (iRet >= 0)
        {
            status = CodeSignStatus::CODE_SIGN_OK;
        }
    }
    PM_LOG_INFO("Package %s codesign verification: status = %d", path.string().c_str(), status);
    return status;
}

CodeSignStatus CodesignVerifier::ExecutableVerify( const std::filesystem::path& path, const std::string& strSigner, SigType sig_type )
{
    if((path.empty()) || (strSigner.empty()) || (SigType::SIGTYPE_NATIVE != sig_type))
    {
        PM_LOG_ERROR("invalid parameters [%ls] : [%ls]", path.string().c_str(), strSigner.c_str());
        return CodeSignStatus::CODE_SIGN_FAIL;
    }
    return ExecutableVerifyWithKilldate( path.string().c_str(), strSigner.c_str(), sig_type, KILLDATE );
}

CodeSignStatus CodesignVerifier::PackageVerify( const std::filesystem::path& path )
{
    bool has_valid_signature{true};
    
    const auto error_code = pkg_verify(path.string().c_str(), &has_valid_signature);
    
    if (error_code != PKG_SIGN_SUCCESS)
        return CodeSignStatus::CODE_SIGN_FAIL;
    
    if (!has_valid_signature)
        return CodeSignStatus::CODE_SIGN_VERIFICATION_FAILED;
    
    return CodeSignStatus::CODE_SIGN_OK;
}

CodeSignStatus CodesignVerifier::PackageVerify( const std::filesystem::path& path, const std::string& expectedSigner )
{
    PM_LOG_INFO("Verify package %s code signing", path.c_str());
    const auto error_code = pkg_verify_trusted_signer(path.string().c_str(), expectedSigner.c_str());
    switch (error_code) {
    case PKG_SIGN_SUCCESS:
        PM_LOG_INFO("Package %s has valid codesigning by %s", path.string().c_str(), expectedSigner.c_str());
        return CodeSignStatus::CODE_SIGN_OK;
    case PKG_SIGN_EDSIG:
        PM_LOG_WARNING("Package %s has no valid codesigning", path.string().c_str());
        return CodeSignStatus::CODE_SIGN_VERIFICATION_FAILED;
    default:
        PM_LOG_ERROR("Failed to detect package %s codesigning", path.string().c_str());
        return CodeSignStatus::CODE_SIGN_FAIL;
    }
}

