#pragma once

#include <unistd.h>

#define SIGNER_CISCO_CN     "Cisco Systems, Inc."
#define SIGNER_CISCO_ENT    "Developer ID Installer: Cisco (DE8Y96K9QP)"
#define SIGNER_MICROSOFT    "Microsoft Corporation"
#define SIGNER_MOZILLA      "Mozilla Corporation"

#define SIGNER_CISCO_BIN SIGNER_CISCO_TEAMID
#define SIGNER_CISCO_PKG SIGNER_CISCO_ENT
#define SIGNER_CISCO_TEAMID "DE8Y96K9QP"

#define KILLDATE ((unsigned long long)1337659200) // TODO: it is epoch of 22 May 2012, some magic number?

enum class SigType
{
    SIGTYPE_AUTO = 0,
    SIGTYPE_NATIVE,
    SIGTYPE_CISCO
};
#define SIGTYPE_DEFAULT     SigType::SIGTYPE_NATIVE

/**
 * @brief Checks if the specified file's signature is valid.
 *
 * This function verifies whether the signature of the given file is valid.
 *
 * @param path Path to the file whose signature needs to be checked.
 * @return Returns 0 if the signature is valid, otherwise returns -1.
 */
int check_signature(const char* path);

/**
 * @brief Checks if the specified file was signed by a specific signer with given conditions.
 *
 * This function verifies whether the specified file was signed by a given signer and meets
 * the provided conditions for the signature type and killdate.
 *
 * @param path Path to the file whose signature needs to be verified.
 * @param signer The expected signer's identifier.
 * @param sig_type The type of the signature.
 * @param killdate A timestamp indicating the expiration date for the signature.
 * @return Returns 0 if the signature and conditions are met, otherwise returns -1.
 */
int check_signer(const char* path, const char* signer, SigType sig_type, uint64_t killdate);


