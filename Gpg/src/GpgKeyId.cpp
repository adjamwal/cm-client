/**
 * @file
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved.
 */
#include "GpgKeyId.hpp"

#include <algorithm>
#include <locale>
#include <ostream>

GpgKeyId::GpgKeyId() noexcept
{
}

GpgKeyId::GpgKeyId(const std::string &key) : key_{ key }
{
    std::transform(key_.begin(), key_.end(), key_.begin(), ::tolower);
}

bool GpgKeyId::is_valid() const
{
    return key_.length() >= long_key_length;
}

std::string GpgKeyId::long_id() const
{
    if (!is_valid()) {
        return {};
    }
    return key_.substr(key_.length() - long_key_length);
}

bool operator==(const GpgKeyId &lhs, const GpgKeyId &rhs)
{
    /**
     * Note: when comparing GpgKeyId's the keys will not be considered for
     * comparison if either one is invalid. This extra check is done for two
     * reasons:
     * 1. The expected use of the GpgKeyId class is that users should check is_valid
     *    before calling any other GpgKeyId functions.
     * 2. The use-case of the GpgKeyId class is used to determine if a package is
     *    valid. If the GpgKeyId is invalid, then we do not want to mark the
     *    package as trusted, so we must return false to prevent false-positives.
     */
    if (!lhs.is_valid() || !rhs.is_valid()) {
        return false;
    }
    return (lhs.long_id() == rhs.long_id());
}

bool operator==(const GpgKeyId &lhs, const std::string &rhs)
{
    return (lhs == GpgKeyId{ rhs });
}

bool operator==(const std::string &lhs, const GpgKeyId &rhs)
{
    return (GpgKeyId{ lhs } == rhs);
}

bool operator!=(const GpgKeyId &lhs, const GpgKeyId &rhs)
{
    return !(lhs == rhs);
}

bool operator!=(const GpgKeyId &lhs, const std::string &rhs)
{
    return !(lhs == rhs);
}

bool operator!=(const std::string &lhs, const GpgKeyId &rhs)
{
    return !(lhs == rhs);
}

std::ostream &operator<<(std::ostream &os, GpgKeyId key_id) //NOLINT (build fails when applying const to GpgKeyId)
{
    if (!key_id.is_valid()) {
        return os << "(null)";
    }
    return os << key_id.long_id();
}
