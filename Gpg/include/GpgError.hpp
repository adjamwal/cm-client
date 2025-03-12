/**
 * @file
 *
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved.
 */

#pragma once

#include <gpgme.h>
#include <stdexcept>

class GpgError : public std::runtime_error
{
public:
    explicit GpgError(gpgme_error_t error);
};
