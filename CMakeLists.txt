# CMakeLists.txt
# Copyright 2022, Cisco Systems, Inc.

cmake_minimum_required(VERSION 3.19)

# Specify search path for CMake modules to be loaded by include()
# and find_package()
set(CUSTOM_CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

list(APPEND CMAKE_MODULE_PATH "${CUSTOM_CMAKE_MODULE_PATH}")
list(APPEND CMAKE_MODULE_PATH "${CUSTOM_CMAKE_MODULE_PATH}/helper")

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CXX_EXTENSIONS ON) # -std=gnu++11 instead of -std=c++11.

project(cm-client VERSION 1.0.0)

# Include helper functions.
include(determine_architecture)
include(determine_platform)

if(APPLE)
    message("** Building on macOS, generator ${CMAKE_GENERATOR} **")
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15 CACHE STRING "Minimum OS X deployment version" FORCE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Architectures" FORCE)

    #
    # The `;` must be escaped when passed into ExternalProject_Add()
    string(REPLACE ";" "$<SEMICOLON>" CMAKE_OSX_ARCHITECTURES_ "${CMAKE_OSX_ARCHITECTURES}")

    set(XCODE_TOOLCHAIN_BIN "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin")
    set(XCODE_MACOS_SDK "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk")
    set(XCODE_CC "${XCODE_TOOLCHAIN_BIN}/clang -mmacosx-version-min=10.15 -isysroot ${XCODE_MACOS_SDK} -g -arch x86_64 -arch arm64")
    set(XCODE_CPP "${XCODE_TOOLCHAIN_BIN}/clang -mmacosx-version-min=10.15 -isysroot ${XCODE_MACOS_SDK} -E")
    set(XCODE_AR "${XCODE_TOOLCHAIN_BIN}/ar r")
    set(XCODE_NM "${XCODE_TOOLCHAIN_BIN}/nm")
    set(XCODE_RANLIB "${XCODE_TOOLCHAIN_BIN}/ranlib")

    if(DEFINED SIGNING_CERT)
        message("--Build will sign using ${SIGNING_CERT}")
    endif()
endif()

# remove ZERO_CHECK target from xcode
set(CMAKE_SUPPRESS_REGENERATION true)


if(DEFINED BUILD_ALL_THIRD_PARTY AND BUILD_ALL_THIRD_PARTY)
    set(BUILD_ALL_THIRD_PARTY true)
else()
    set(BUILD_ALL_THIRD_PARTY false)
endif()

option(BUILD_DEV_ONLY "" OFF)
# Change manually to force an individual library to be built instead
option(BUILD_PACKAGE_MANAGER_THIRD_PARTY "" OFF)
option(BUILD_ENDPOINT_IDENTITY_THIRD_PARTY "" OFF)
option(BUILD_CRASHPAD "" OFF)
option(BUILD_PROXY_DISCOVERY_THIRD_PARTY "" OFF)

if(BUILD_DEV_ONLY AND (BUILD_ALL_THIRD_PARTY OR BUILD_PACKAGE_MANAGER_THIRD_PARTY))
    set(BUILD_WITH_HTTP_ENABLED ON CACHE STRING INTERNAL FORCE)
endif()

# config.h checks
include(ConfigureChecks.cmake)
configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(CM_SCRIPTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/scripts")
set(CM_THIRDPARTY_EXPORT "${CMAKE_CURRENT_BINARY_DIR}/export")

file(MAKE_DIRECTORY ${CM_THIRDPARTY_EXPORT})

##
## Include Artifactory Helpers
include(ArtifactoryDownload)
include(ArtifactoryUpload)

##
## External Package Definitions for third-party and first-party dependencies
include(ExternalProject_spdlog)
include(ExternalProject_ciscossl)
include(ExternalProject_curl)
include(ExternalProject_jsoncpp)
if(APPLE)
    include(ExternalProject_crashpad)
    include(ExternalProject_ProxyDiscovery-Mac)
else()
    include(ExternalProject_libxml2)
endif()
include(ExternalProject_gtest)
include(ExternalProject_PackageManager)
include(ExternalProject_EndpointIdentity)

# Check if BUILD_WITH_HTTP_ENABLED is set to ON in the PackageManager
if(BUILD_WITH_HTTP_ENABLED)
    message(STATUS "Building with HTTP support")
else()
    message(STATUS "Building without HTTP support")
endif()

##
## util library
add_subdirectory(util)

##
## configshared library (common staff for PM and CM)
add_subdirectory(ConfigShared)

##
## Package Manager client
add_subdirectory(OSPackageManager)

##
## The Cloud Management Connector client
add_subdirectory(client)

if(APPLE)
##
##
add_subdirectory(Uninstaller)
endif()
