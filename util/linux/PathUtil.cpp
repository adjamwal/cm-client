/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */
#include "PathUtil.hpp"

#include <filesystem>

#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

namespace util
{

std::string getExecutablePath()
{
    char path[PATH_MAX] {};
    char program_path[PATH_MAX] {};

    pid_t pid = getpid();
    (void) sprintf(path, "/proc/%d/exe", pid);
    if (readlink(path, program_path, PATH_MAX) == -1) {
      perror("readlink failed to get executable path");
    }

    return std::string { program_path };
}

std::string getApplicationName()
{
    namespace fs = std::filesystem;
    return fs::path(getExecutablePath()).filename();
}

std::string getApplicationVersion()
{
    return std::string { CM_BUILD_VER };
}

} //util

