# crashpad external project build file
# Copyright Cisco Systems, Inc. 2023

include(ExternalProject)

set(component_name ProxyDiscovery-Mac)
set(component_dst_dir "${CMAKE_CURRENT_BINARY_DIR}/third-party-${component_name}-prefix/src/third-party-${component_name}")
if(CMAKE_GENERATOR STREQUAL Xcode)
    set(component_build_folder_prefix "xcode_")
else()
    set(component_build_folder_prefix "")
endif()

set(component_install_prefix "${CMAKE_CURRENT_SOURCE_DIR}/${component_name}/${component_build_folder_prefix}export")

if(NOT BUILD_ALL_THIRD_PARTY AND NOT BUILD_PROXY_DISCOVERY_THIRD_PARTY)
    download_component(${component_name} ${component_dst_dir})
endif()


if(NOT TARGET "third-party-${component_name}")
    ExternalProject_Add(
        third-party-${component_name}
        DEPENDS third-party-gtest
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${component_name}/
        PREFIX ${component_install_prefix}
        CMAKE_GENERATOR ${CMAKE_GENERATOR}
        CMAKE_ARGS
            -Dgtest_INCLUDE_DIRS=${CM_THIRDPARTY_EXPORT}/include
            -Dgtest_LIBRARY=${CM_THIRDPARTY_EXPORT}/lib
            -DSKIP_TESTS=FALSE
            -DCMAKE_INSTALL_PREFIX:PATH=${component_install_prefix}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES_}
            -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
    )

    upload_component(${component_name} not_used)
endif()
