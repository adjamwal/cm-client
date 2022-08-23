# Google Test external project build file
# Copyright Cisco Systems, Inc. 2022

include(ExternalProject)

set(GTEST_EXPORT_DIR "{CMAKE_CURRENT_SOURCE_DIR}/third-party/gtest/export")

#
# TODO Be more specific about tool chain
ExternalProject_Add(
    third-party-gtest
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-party/gtest/googletest-release-1.10.0
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/third-party/gtest
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX:PATH=${GTEST_EXPORT_DIR}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES_}
        -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
)
