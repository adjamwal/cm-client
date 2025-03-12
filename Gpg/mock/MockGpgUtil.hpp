/**
 * @file
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved.
 */
#pragma once

#include "Gpg/include/IGpgUtil.hpp"
#include "Gpg/include/GpgKeyId.hpp"
#include "gmock/gmock.h"

#include <string>
class GpgKeyId;

class MockGpgUtil : public IGpgUtil
{
    public:
        MOCK_METHOD(GpgKeyId, get_signature_fingerprint, (const std::string &path), (override));
        MOCK_METHOD(GpgKeyId, get_pubkey_fingerprint, (const std::vector<char> &key), (override));
        MOCK_METHOD(bool, validate_signature,
                (const std::vector<char> &key_path, const std::vector<char> &data, const std::list<std::filesystem::path> &keys),
            (override));
};
