# Determine the current platform. e.g. ubuntu20.04
# Copyright Cisco Systems, Inc. 2024

include(determine_architecture)

# LINUX is defined in CMake version 3.25
if(NOT DEFINED LINUX)
    if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
        SET(LINUX TRUE)
    else()
        SET(LINUX FALSE)
    endif()
endif()

# APPLE does not seem to be defined despite existing in documentation
if(NOT DEFINED APPLE)
    if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
        SET(APPLE TRUE)
    else()
        SET(APPLE FALSE)
    endif()
endif()

if(NOT APPLE AND NOT LINUX)
    message(FATAL_ERROR "Error: System is neither APPLE nor LINUX")
endif()


if(LINUX)
    set(RHEL_VARIANTS
        "centos"
        "redhat"
        "rocky"
        "almalinux"
        "oracle"
        "rhel")

    cmake_host_system_information(RESULT DISTRO QUERY DISTRIB_INFO)

    if(${DISTRO_ID} IN_LIST RHEL_VARIANTS)
        string(SUBSTRING "${DISTRO_VERSION}" 0 1 DISTRO_VERSION_SHORT)
        set(platform "el${DISTRO_VERSION_SHORT}")
        add_definitions(-DIS_RHEL)
    else()
        string(SUBSTRING "${DISTRO_VERSION}" 0 2 DISTRO_VERSION_SHORT)
        string(FIND ${DISTRO_ID} "suse" is_suse)
        string(FIND ${DISTRO_ID} "sles" is_sles)
        if((NOT is_suse MATCHES -1) OR (NOT is_sles MATCHES -1))
            set(platform "suse${DISTRO_VERSION_SHORT}")
        else()
            set(platform "${DISTRO_ID}${DISTRO_VERSION_SHORT}")
        endif()
    endif()
elseif(APPLE)
    set(platform "macOS")
endif()

string(STRIP "${platform}" platform)

# Environment variable only exists during configure stage.
set(ENV{PLATFORM} ${platform})
