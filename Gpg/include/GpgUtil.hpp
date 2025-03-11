/**
 * @file
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved.
 */
#pragma once

#include "IGpgUtil.hpp"

#include <string>
class GpgKeyId;

class GpgUtil : public IGpgUtil
{
public:
    GpgUtil() noexcept;
    static GpgUtil &instance();

    GpgKeyId get_signature_fingerprint(const std::string &path) override;
    bool validate_signature(const std::vector<char> &signature, const std::vector<char> &data, const std::list<std::filesystem::path> &keys) override;
    GpgKeyId get_pubkey_fingerprint(const std::vector<char> &key) override;
};
