/**
 * @file
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved.
 */

#include "GpgUtil.hpp"

#include <filesystem>
#include <string>
#include "TemporaryDirectory.hpp"
#include "GpgContext.hpp"
#include "GpgError.hpp"
#include "GpgKeyId.hpp"

GpgUtil::GpgUtil() noexcept
{
    gpgme_check_version(nullptr);
}

GpgUtil &GpgUtil::instance()
{
    static GpgUtil instance;
    return instance;
}

GpgKeyId GpgUtil::get_signature_fingerprint(const std::string &path)
{
    try {
        GpgContext context{};
        return context.signature_fingerprint(path);
    } catch (const GpgError &error) {
        //fm_error(FAC_UTIL, "Failed to get signature key: %s", error.what());
        return {};
    }
}

bool GpgUtil::validate_signature(const std::vector<char> &signature,
                                 const std::vector<char> &data,
                                 const std::list<std::filesystem::path> &keys)
{
    try {
        GpgContext context{};
        TemporaryDirectory gnupg_dir{};
        context.set_home(gnupg_dir.path());

        for (auto const &key : keys) {
            context.import(key);
        }

        return context.verify(signature, data);
    } catch (const GpgError &error) {
        //fm_error(FAC_UTIL, "Failed to validate signature: %s", error.what());
        return false;
    }
}

GpgKeyId GpgUtil::get_pubkey_fingerprint(const std::vector<char> &key)
{
    try {
        GpgContext context{};
        TemporaryDirectory gnupg_dir{};
        context.set_home(gnupg_dir.path());

        return context.pubkey_fingerprint(key);
    } catch (const GpgError &error) {
        //fm_error(FAC_UTIL, "Failed to get public key fingerprint: %s", error.what());
        return {};
    }
}
