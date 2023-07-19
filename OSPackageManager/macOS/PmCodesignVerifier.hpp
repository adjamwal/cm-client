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

#define SIGNER_CISCO_CN     "Cisco Systems, Inc."
#define SIGNER_MICROSOFT    "Microsoft Corporation"
#define SIGNER_MOZILLA      "Mozilla Corporation"

#define SIGNER_CISCO_TEAMID "DE8Y96K9QP"
#define SIGNER_CISCO SIGNER_CISCO_TEAMID

class CodesignVerifier : public IPmCodesignVerifier
{
public:
    CodeSignStatus Verify( const std::string& strPath, const std::string& strSigner, SigType sig_type ) override;
    CodeSignStatus VerifyWithKilldate( const std::string& strPath, const std::string& strSigner,
                                      SigType sig_type, uint64_t killdate ) override;
};
