# jsoncpp external project build file
# Copyright Cisco Systems, Inc. 2022

include(ExternalProject)

#
# TODO Be more specific about tool chain

ExternalProject_Add(
    third-party-jsoncpp
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-party/jsoncpp/jsoncpp
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/jsoncpp/export
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_SOURCE_DIR}/third-party/jsoncpp/export
)
