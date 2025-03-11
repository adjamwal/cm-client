/**
 * @file
 *
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved.
 */

#include <iostream>
#include "GpgKeychain.hpp"

GpgKeychain::GpgKeychain(const std::filesystem::path &keychain_path) noexcept
{
    if (!std::filesystem::exists(keychain_path) || std::filesystem::is_empty(keychain_path)) {
        return;
    }

    /** Note: following debsig-verify:
     * - keyfile must be at a depth of 1 in the keychain
     * - keyfile must be a regular file
     * - keyfile must end in .gpg extension
     * - symlinks are allowed, but they must end in .gpg extension and point directly to keyfile
     */
    for (auto it = std::filesystem::recursive_directory_iterator(keychain_path, std::filesystem::directory_options::follow_directory_symlink);
         it != std::filesystem::recursive_directory_iterator();
         ++it) {
        if (it.depth() != 1) {
            continue;
        }

        const std::filesystem::path gpg_extension { ".gpg" };
        if (std::filesystem::is_regular_file(it->path()) && it->path().extension() == gpg_extension) {
            keys_.push_back(it->path());
        }
    }
}

std::list<std::filesystem::path> GpgKeychain::keys()
{
    return keys_;
}
