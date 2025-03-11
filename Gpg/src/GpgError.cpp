/**
 * @file
 *
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved.
 */

#include "GpgError.hpp"

GpgError::GpgError(gpg_error_t error) : std::runtime_error{ gpgme_strerror(error) }
{
}
