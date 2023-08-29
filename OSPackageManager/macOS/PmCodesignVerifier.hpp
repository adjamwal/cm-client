/**************************************************************************
 * Copyright (c) 2020, Cisco Systems, All Rights Reserved
 ***************************************************************************
 *
 * @file : PmCodesignVerifier.h
 *
 ***************************************************************************
 *
 * Defines file signature verification
 *
 ***************************************************************************/

#pragma once

#include "IPmCodesignVerifier.hpp"
#include "IPmPkgUtil.hpp"

class CodesignVerifier : public IPmCodesignVerifier
{
public:
    explicit CodesignVerifier(std::shared_ptr<IPmPkgUtil>);
    CodeSignStatus ExecutableVerify( const std::filesystem::path& path, const std::string& strSigner, SigType sig_type ) override;
    CodeSignStatus ExecutableVerifyWithKilldate( const std::filesystem::path& path, const std::string& strSigner,
                                      SigType sig_type, uint64_t killdate ) override;
    CodeSignStatus PackageVerify( const std::filesystem::path& path ) override;
    CodeSignStatus PackageVerify( const std::filesystem::path& path, const std::string& strSigner ) override;
    
private:
    std::shared_ptr<IPmPkgUtil> pkgUtil_;
};
