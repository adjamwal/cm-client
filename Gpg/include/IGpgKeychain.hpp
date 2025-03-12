/**
 * @file
 *
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved.
 */

#pragma once

#include <filesystem>
#include <list>

class IGpgKeychain
{
public:
    IGpgKeychain() noexcept = default;
    virtual ~IGpgKeychain() = default;
    IGpgKeychain(const IGpgKeychain &other) = delete;
    IGpgKeychain &operator=(const IGpgKeychain &other) = delete;
    IGpgKeychain(IGpgKeychain &&other) = delete;
    IGpgKeychain &operator=(IGpgKeychain &&other) = delete;

    virtual std::list<std::filesystem::path> keys() = 0;
};
