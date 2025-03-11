/**
 * @file
 *
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved.
 */

#pragma once

#include <filesystem>
#include <gpgme.h>
#include <string>
#include <vector>

class GpgKeyId;

class GpgContext
{
public:
    GpgContext();
    ~GpgContext();
    GpgContext(const GpgContext &other) = delete;
    GpgContext &operator=(const GpgContext &other) = delete;
    GpgContext(GpgContext &&other) = delete;
    GpgContext &operator=(GpgContext &&other) = delete;

    void set_home(const std::filesystem::path &home);
    void import(const std::filesystem::path &key);
    GpgKeyId pubkey_fingerprint(const std::vector<char> &key);

    GpgKeyId signature_fingerprint(const std::string &signature);

    bool verify(const std::vector<char> &signature, const std::vector<char> &data);

private:
    gpgme_ctx_t context_ = nullptr;
};
