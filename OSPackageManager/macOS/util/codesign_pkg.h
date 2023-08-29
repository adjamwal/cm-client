#pragma once

#include <stdio.h>
#include <stdbool.h>
    
typedef enum {
    PKG_SIGN_EUNKNOWN,
    PKG_SIGN_ENULLARG,
    PKG_SIGN_EARG,
    PKG_SIGN_EACCES,
    PKG_SIGN_EDSIG,
    PKG_SIGN_EMEM,
    PKG_SIGN_EOPEN,
    PKG_SIGN_ESTAT,
    PKG_SIGN_SUCCESS
} pkg_sign_error_t;

/**
 * @brief Verify software package.
 *
 * @param[in] package_path - Absolute or relative path to software package
 * @param[out] is_signed - Pointer to boolean that is set to True if package is
 *                         signed
 *
 * @return PKG_SIGN_SUCCESS on success or PKG_SIGN_E* on error
 */
pkg_sign_error_t pkg_verify(const char *package_path, bool *is_signed);

/**
 * @brief Verify software package and check signing against a specific trusted signer
 *
 * @param[in] package_path - Absolute or relative path to software package
 * @param[in] trusted_signer - Signing certificate used to sign the package
 *
 * @return PKG_SIGN_SUCCESS on success, PKG_SIGN_E* or error
 */
pkg_sign_error_t pkg_verify_trusted_signer(const char *package_path, const char * const trusted_signer);

