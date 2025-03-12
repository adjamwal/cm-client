/**
 * @file
 *
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved.
 */

#include "GpgContext.hpp"

#include "GpgError.hpp"
#include "GpgKeyId.hpp"

namespace
{
class GpgData
{
public:
    explicit GpgData(const std::filesystem::path &file_path);
    explicit GpgData(const std::vector<char> &data);
    ~GpgData();
    GpgData(const GpgData &other) = delete;
    GpgData &operator=(const GpgData &other) = delete;
    GpgData(GpgData &&other) = delete;
    GpgData &operator=(GpgData &&other) = delete;

    gpgme_data_t get() const;

private:
    gpgme_data_t data_;
};
} // namespace

GpgData::GpgData(const std::filesystem::path &file_path)
{
    static const int copy_data = 1;
    const gpgme_error_t error = gpgme_data_new_from_file(&data_, file_path.c_str(), copy_data);
    if (error) {
        throw GpgError{ error };
    }
}

GpgData::GpgData(const std::vector<char> &data)
{
    static const int copy_data = 1;
    const gpgme_error_t error = gpgme_data_new_from_mem(&data_, data.data(), data.size(), copy_data);
    if (error) {
        throw GpgError{ error };
    }
}

GpgData::~GpgData()
{
    gpgme_data_release(data_);
}

gpgme_data_t GpgData::get() const
{
    return data_;
}

GpgContext::GpgContext()
{
    const gpgme_error_t context_error = gpgme_new(&context_);
    if (context_error) {
        throw GpgError(context_error);
    }

    const gpgme_error_t protocol_error = gpgme_set_protocol(context_, GPGME_PROTOCOL_OpenPGP);
    if (protocol_error) {
        throw GpgError(protocol_error);
    }
}

GpgContext::~GpgContext()
{
    gpgme_release(context_);
}

void GpgContext::set_home(const std::filesystem::path &home)
{
    gpgme_engine_info_t info = gpgme_ctx_get_engine_info(context_);
    const gpgme_error_t error =
        gpgme_ctx_set_engine_info(context_, info->protocol, info->file_name, home.c_str());
    if (error) {
        throw GpgError{ error };
    }
}

void GpgContext::import(const std::filesystem::path &key)
{
    GpgData key_data{ key };

    const gpgme_error_t error = gpgme_op_import(context_, key_data.get());
    if (error) {
        throw GpgError{ error };
    }
}

GpgKeyId GpgContext::pubkey_fingerprint(const std::vector<char> &key)
{
    GpgData key_data{ key };

    const gpgme_error_t error = gpgme_op_import(context_, key_data.get());
    if (error) {
        throw GpgError{ error };
    }

    gpgme_import_result_t iresult = gpgme_op_import_result(context_);
    if (iresult->imports) {
        return GpgKeyId{ iresult->imports->fpr };
    }
    return GpgKeyId{};
}

GpgKeyId GpgContext::signature_fingerprint(const std::string &signature)
{
    GpgData signature_data{ signature };

    const gpgme_error_t error = gpgme_op_verify(context_, signature_data.get(), nullptr, nullptr);
    if (error) {
        throw GpgError{ error };
    }

    gpgme_verify_result_t vresult = gpgme_op_verify_result(context_);
    if (!vresult || !vresult->signatures || !vresult->signatures->fpr) {
        return {};
    }
    return GpgKeyId{ vresult->signatures->fpr };
}

bool GpgContext::verify(const std::vector<char> &signature, const std::vector<char> &data)
{
    GpgData signature_data{ signature };
    GpgData signed_data{ data };

    const gpgme_error_t error = gpgme_op_verify(context_, signature_data.get(), signed_data.get(), nullptr);
    if (error) {
        throw GpgError{ error };
    }

    gpgme_verify_result_t result = gpgme_op_verify_result(context_);
    if (!result || !result->signatures) {
        return false;
    }

    const gpgme_sigsum_t summary = result->signatures->summary;
    /* When summary is set to 0 the signature is valid but the key is unknown. Since we are automatically
     * importing the key into a temporary keychain there is no way for us to set the key to trusted as it
     * requires user intervention (prompt). */
    return !(summary & ~(GPGME_SIGSUM_VALID | GPGME_SIGSUM_GREEN | GPGME_SIGSUM_KEY_EXPIRED));
}
