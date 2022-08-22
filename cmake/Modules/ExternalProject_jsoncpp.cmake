# jsoncpp external project build file
# Copyright Cisco Systems, Inc. 2022

include(ExternalProject)

#
# The `;` must be escaped when passed into ExternalProject_Add()
string(REPLACE ";" "$<SEMICOLON>" CMAKE_OSX_ARCHITECTURES_ "${CMAKE_OSX_ARCHITECTURES}")

#
# TODO Be more specific about tool chain
# TODO Adapt to make OSX options based on target
ExternalProject_Add(
    third-party-jsoncpp
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-party/jsoncpp/jsoncpp
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/jsoncpp/export
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_SOURCE_DIR}/third-party/jsoncpp/export
        -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES_}
        -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
)
