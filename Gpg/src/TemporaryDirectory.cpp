/**
 * @file
 *
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved.
 */

#include "TemporaryDirectory.hpp"

#include <algorithm>
#include <random>

TemporaryDirectory::TemporaryDirectory()
    : dir_name_len_ { 10 }
    , path_{ create_directory() }
{
}

TemporaryDirectory::TemporaryDirectory(size_t depth, size_t dir_name_len)
    : dir_name_len_ { dir_name_len }
{
    std::filesystem::path path{ std::filesystem::temp_directory_path() };
    for (size_t i = 1; i < depth; ++i) { // Start at depth 1.
        path = create_directory(path);
    }
    path_ = path;
}

TemporaryDirectory::TemporaryDirectory(const std::filesystem::path &path, size_t dir_name_len) 
    : dir_name_len_ { dir_name_len }
    , path_{ path }
{
    if (!std::filesystem::exists(path)) {
        if (!std::filesystem::create_directory(path)) {
            throw std::runtime_error("Failed to create directory: " + path.string());
        }
    }
}

const std::filesystem::path TemporaryDirectory::create_directory(const std::filesystem::path& base_directory) const
{
    static constexpr int max_attempts{ 10 };
    for (int i = 0; i < max_attempts; ++i) {
        const std::filesystem::path path { base_directory / gen_file_name(dir_name_len_) };
        if (std::filesystem::create_directory(path)) {
            return path;
        }
    }
    throw std::runtime_error("Failed to create temp directory");
}

TemporaryDirectory::~TemporaryDirectory()
{
    reset();
}

const std::filesystem::path &TemporaryDirectory::path() const
{
    return path_;
}

std::string TemporaryDirectory::gen_file_name(std::string::size_type size) const
{
    std::random_device engine;
    auto random_char = [&engine]() {
        /*
         * Care should be taken as to not include a null-byte in the randomly generated file name as it is
         * known to cause issues in std::filesystem. Specifically std::filesystem::remove_all which will get
         * stuck in an infinite loop.
         *
         * Ref: https://bugs.launchpad.net/ubuntu/+source/gcc-8/+bug/1792570
         */
        static const std::string characters{
            "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
        };
        std::uniform_int_distribution distribution{ std::string::size_type{ 0 }, characters.size() - 1 };
        return characters[distribution(engine)];
    };

    std::string file_name{};
    file_name.resize(size, 0);
    std::generate_n(std::begin(file_name), size, random_char);
    return file_name;
}

void TemporaryDirectory::reset() noexcept
{
    std::error_code error{};
    std::filesystem::remove_all(path_, error);
}
