# Google Test external project build file
# Copyright Cisco Systems, Inc. 2022

include(ExternalProject)

set(component_name gtest)
set(component_dst_dir "${CMAKE_CURRENT_BINARY_DIR}/third-party-${component_name}-prefix/src/third-party-${component_name}")
set(component_install_prefix "${CMAKE_CURRENT_SOURCE_DIR}/third-party/${component_name}/export")

set(GTEST_LIBS
    gtest_maind
    gtestd
)

if(NOT BUILD_ALL_THIRD_PARTY)
    download_component(${component_name} ${component_dst_dir})
endif()

if(NOT TARGET "third-party-${component_name}")
    #
    # TODO Be more specific about tool chain
    ExternalProject_Add(
        third-party-${component_name}
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-party/gtest/googletest-release-1.10.0
        PREFIX ${component_install_prefix}
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX:PATH=${component_install_prefix}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES_}
            -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
    )

    upload_component(${component_name} not_used)

    if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(GTEST_LIBS
            gtest_main
            gtest
        )
    endif()
endif()
