# PackageManager external project build file
# Copyright Cisco Systems, Inc. 2022

include(ExternalProject)

set(FFF_EXPORT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party/fff")

#
# TODO Be more specific about tool chain
ExternalProject_Add(
    third-party-PackageManager
    DEPENDS third-party-ciscossl third-party-curl third-party-jsoncpp third-party-gtest
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/PackageManager/cmake
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/third-party/PackageManager/export
    CMAKE_ARGS
        -G ${CMAKE_GENERATOR}
        -Djsoncpp_INCLUDE_DIRS=${CM_THIRDPARTY_EXPORT}/include
        -Djsoncpp_LIBRARY=${CM_THIRDPARTY_EXPORT}/lib
        -Dcurl_INCLUDE_DIRS=${CM_THIRDPARTY_EXPORT}/include
        -Dcurl_LIBRARY=${CM_THIRDPARTY_EXPORT}/lib
        -Dssl_INCLUDE_DIRS=${CM_THIRDPARTY_EXPORT}/include
        -Dssl_LIBRARY=${CM_THIRDPARTY_EXPORT}/lib
        -Dfff_INCLUDE_DIRS=${FFF_EXPORT_DIR}
        -Dgtest_INCLUDE_DIRS=${CM_THIRDPARTY_EXPORT}/include
        -Dgtest_LIBRARY=${CM_THIRDPARTY_EXPORT}/lib
        -DSKIP_TESTS=FALSE
        -DCMAKE_INSTALL_PREFIX:PATH=${CM_THIRDPARTY_EXPORT}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES_}
        -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
)
