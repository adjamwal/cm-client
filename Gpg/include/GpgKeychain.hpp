/**
 * @file
 *
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved.
 */

#pragma once

#include "IGpgKeychain.hpp"

class GpgKeychain : public IGpgKeychain
{
public:
    explicit GpgKeychain(const std::filesystem::path &keychain_path) noexcept;

    std::list<std::filesystem::path> keys() override;

private:
    std::list<std::filesystem::path> keys_;
};
