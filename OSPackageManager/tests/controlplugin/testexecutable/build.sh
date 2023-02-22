#!/bin/sh -e
# Copyright 2023 Cisco Systems, Inc.

CMAKE_EXTRA_ARGS="-DCMAKE_BUILD_TYPE=Debug"
CMAKE_BUILD_DIR="debug"

mkdir -p "${CMAKE_BUILD_DIR}"
pushd "${CMAKE_BUILD_DIR}"
cmake ${CMAKE_EXTRA_ARGS} ../ .
cmake --build .
popd

echo
echo "** Build completed **"
echo
echo " - build directory:	./${CMAKE_BUILD_DIR}"