# CiscoSSL external project build file
# Copyright Cisco Systems, Inc. 2022

include(ExternalProject)

set(component_name ciscossl)
set(component_dst_dir "${CMAKE_CURRENT_BINARY_DIR}/third-party-${component_name}-prefix/src/third-party-${component_name}")
set(component_install_prefix "${CMAKE_CURRENT_SOURCE_DIR}/third-party/${component_name}/export")

if(NOT BUILD_ALL_THIRD_PARTY)
    download_component(${component_name} ${component_dst_dir})
endif()

if(NOT TARGET "third-party-${component_name}")
    #
    # TODO Be more specific about tool chain
    ExternalProject_Add(
        third-party-${component_name}
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND echo "No configuration necessary."
        BUILD_COMMAND
            COMMAND
                #
                # Passing argument -c to do a clean prior to build
                CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                sh "${CMAKE_CURRENT_SOURCE_DIR}/scripts/build_ciscossl.sh" -c
            COMMAND
                CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                sh "${CMAKE_CURRENT_SOURCE_DIR}/scripts/build_ciscossl.sh"
        INSTALL_COMMAND echo "Auto-installed by build"
    )

    upload_component(${component_name} not_used)
endif()

ExternalProject_Add_Step(third-party-${component_name} post_install_symlink_fix
    WORKING_DIRECTORY "${component_dst_dir}/libshared"
    DEPENDEES install
    COMMAND rm -rf libcrypto.dylib
    COMMAND rm -rf libssl.dylib
    COMMAND ln -sf libcrypto.1.1.dylib libcrypto.dylib
    COMMAND ln -sf libssl.1.1.dylib libssl.dylib
)
