# CMakeLists.txt
# Copyright 2022, Cisco Systems, Inc.

cmake_minimum_required(VERSION 3.19)

# Specify search path for CMake modules to be loaded by include()
# and find_package()
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

project(cm-client VERSION 1.0.0)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

if(APPLE)
    message("** Building on macOS, generator ${CMAKE_GENERATOR} **")
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15 CACHE STRING "Minimum OS X deployment version" FORCE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Architectures" FORCE)

    #
    # The `;` must be escaped when passed into ExternalProject_Add()
    string(REPLACE ";" "$<SEMICOLON>" CMAKE_OSX_ARCHITECTURES_ "${CMAKE_OSX_ARCHITECTURES}")
endif()

# remove ZERO_CHECK target from xcode
set(CMAKE_SUPPRESS_REGENERATION true)

# config.h checks
include(ConfigureChecks.cmake)
configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(CM_THIRDPARTY_EXPORT "${CMAKE_CURRENT_BINARY_DIR}/export")

##
## External Package Definitions for third-party and first-party dependencies
include(ExternalProject_ciscossl)
include(ExternalProject_curl)
include(ExternalProject_jsoncpp)
include(ExternalProject_gtest)
include(ExternalProject_PackageManager)
include(ExternalProject_cmid)

##
## Package Manager client
add_subdirectory(OSPackageManager)

##
## The Cloud Management Connector client
add_subdirectory(client)
