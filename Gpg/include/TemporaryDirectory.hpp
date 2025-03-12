/**
 * @file
 *
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved.
 */

#pragma once

#include <filesystem>

class TemporaryDirectory
{
public:
    /**
     * @throw All constructors throw runtime_error on failure to construct.
     */
    TemporaryDirectory();
    TemporaryDirectory(size_t depth, size_t dir_name_len = 10);
    TemporaryDirectory(const std::filesystem::path &path, size_t dir_name_len = 10);

    ~TemporaryDirectory();
    TemporaryDirectory(const TemporaryDirectory &other) = delete;
    TemporaryDirectory &operator=(const TemporaryDirectory &other) = delete;
    TemporaryDirectory(TemporaryDirectory &&other) = default;
    TemporaryDirectory &operator=(TemporaryDirectory &&other) = default;

    const std::filesystem::path &path() const;

private:
    const std::filesystem::path create_directory(const std::filesystem::path& base_directory = std::filesystem::temp_directory_path()) const;
    std::string gen_file_name(std::string::size_type size) const;

    void reset() noexcept;

    size_t dir_name_len_;
    std::filesystem::path path_;
};
