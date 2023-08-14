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

#define SIGNER_CISCO_CN     "Cisco Systems, Inc."
#define SIGNER_CISCO_ENT    "Cisco"
#define SIGNER_MICROSOFT    "Microsoft Corporation"
#define SIGNER_MOZILLA      "Mozilla Corporation"

#define SIGNER_CISCO_BIN SIGNER_CISCO_TEAMID
#define SIGNER_CISCO_PKG SIGNER_CISCO_ENT
#define SIGNER_CISCO_TEAMID "DE8Y96K9QP"

class CodesignVerifier : public IPmCodesignVerifier
{
public:
    explicit CodesignVerifier(std::shared_ptr<IPmPkgUtil>);
    CodeSignStatus ExecutableVerify( const std::filesystem::path& path, const std::string& strSigner, SigType sig_type ) override;
    CodeSignStatus ExecutableVerifyWithKilldate( const std::filesystem::path& path, const std::string& strSigner,
                                      SigType sig_type, uint64_t killdate ) override;
    CodeSignStatus PackageVerify( const std::filesystem::path& path, const std::string& strSigner ) override;
    
private:
    std::shared_ptr<IPmPkgUtil> pkgUtil_;
};
