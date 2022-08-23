# PackageManager external project build file
# Copyright Cisco Systems, Inc. 2022

include(ExternalProject)

set(FFF_EXPORT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party/fff")

set(PACKGE_MANAGER_EXPORT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party/PackageManager/export")

#
# TODO Be more specific about tool chain
ExternalProject_Add(
    third-party-PackageManager
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/PackageManager/cmake
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/third-party/PackageManager/export
    CMAKE_ARGS
        -Djsoncpp_INCLUDE_DIRS=${JSONCPP_EXPORT_DIR}/include
        -Djsoncpp_LIBRARY=${JSONCPP_EXPORT_DIR}/lib
        -Dcurl_INCLUDE_DIRS=${CURL_EXPORT_DIR}/include
        -Dcurl_LIBRARY=${CURL_EXPORT_DIR}/lib
        -Dssl_INCLUDE_DIRS=${CISCOSSL_EXPORT_DIR}/include
        -Dssl_LIBRARY=${CISCOSSL_EXPORT_DIR}/lib
        -Dfff_INCLUDE_DIRS=${FFF_EXPORT_DIR}
        -Dgtest_INCLUDE_DIRS=${GTEST_EXPORT_DIR}/include
        -Dgtest_LIBRARY=${GTEST_EXPORT_DIR}/lib
        -DCMAKE_INSTALL_PREFIX:PATH=${PACKAGE_MANAGER_EXPORT_DIR}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES_}
        -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
)
