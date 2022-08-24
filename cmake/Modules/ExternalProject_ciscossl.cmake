# CiscoSSL external project build file
# Copyright Cisco Systems, Inc. 2022

include(ExternalProject)

#
# TODO Be more specific about tool chain
ExternalProject_Add(
    third-party-ciscossl
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND echo "No configuration necessary."
    BUILD_COMMAND sh "${CMAKE_CURRENT_SOURCE_DIR}/scripts/build_ciscossl.sh"
    INSTALL_COMMAND echo "Auto-installed by build"
)
