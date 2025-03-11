/**
 * @file
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved.
 */
#pragma once

#include <cstddef>
#include <iosfwd>
#include <string>

#define LONG_KEY_LEN 16

class GpgKeyId final
{
public:
    GpgKeyId() noexcept;
    explicit GpgKeyId(const std::string &key);

    /**
     * @brief Check if the key ID is valid. Users should check this before calling any other functions.
     *
     * @return true if key ID is valid, false otherwise
     */
    bool is_valid() const;

    /**
     * @brief Return key ID in long 16 byte form.
     *
     * @return long key ID
     */
    std::string long_id() const;

private:
    friend bool operator==(const GpgKeyId &lhs, const GpgKeyId &rhs);
    friend std::ostream &operator<<(std::ostream &os, GpgKeyId key_id);

    static constexpr std::size_t long_key_length = LONG_KEY_LEN;

    std::string key_;
};

bool operator==(const GpgKeyId &lhs, const GpgKeyId &rhs);
bool operator==(const GpgKeyId &lhs, const std::string &rhs);
bool operator==(const std::string &lhs, const GpgKeyId &rhs);
bool operator!=(const GpgKeyId &lhs, const GpgKeyId &rhs);
bool operator!=(const GpgKeyId &lhs, const std::string &rhs);
bool operator!=(const std::string &lhs, const GpgKeyId &rhs);

std::ostream &operator<<(std::ostream &os, GpgKeyId key_id);
