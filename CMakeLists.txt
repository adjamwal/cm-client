# CMakeLists.txt
# Copyright 2022, Cisco Systems, Inc.

cmake_minimum_required(VERSION 3.6.1)

# Specify search path for CMake modules to be loaded by include()
# and find_package()
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

project(cm-client VERSION 1.0.0)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

# remove ZERO_CHECK target from xcode
set(CMAKE_SUPPRESS_REGENERATION true)

# define DEBUG for all debug builds
set(CMAKE_C_FLAGS_DEBUG "-DDEBUG")

# config.h checks
include(ConfigureChecks.cmake)
configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

include(ExternalProject_jsoncpp)

add_subdirectory(client)
