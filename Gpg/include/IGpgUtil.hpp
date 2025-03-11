/**
 * @file
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved.
 */
#pragma once

#include <filesystem>
#include <list>
#include <string>
#include <vector>

class GpgKeyId;

class IGpgUtil
{
public:
    IGpgUtil() noexcept = default;
    virtual ~IGpgUtil() = default;
    IGpgUtil(const IGpgUtil &other) = delete;
    IGpgUtil &operator=(const IGpgUtil &other) = delete;
    IGpgUtil(IGpgUtil &&other) = delete;
    IGpgUtil &operator=(IGpgUtil &&other) = delete;

    /**
     * @brief Return key ID used to sign the given signature file
     *
     * @param[in] path Path to GPG signature file
     *
     * @return Key ID
     */
    virtual GpgKeyId get_signature_fingerprint(const std::string &path) = 0;
    virtual GpgKeyId get_pubkey_fingerprint(const std::vector<char> &key) = 0;
    virtual bool validate_signature(const std::vector<char> &signature,
                                    const std::vector<char> &data,
                                    const std::list<std::filesystem::path> &keys) = 0;
};
