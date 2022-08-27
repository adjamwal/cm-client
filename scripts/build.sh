#!/bin/sh -e
# Copyright 2022 Cisco Systems, Inc.

SYSTEM="$(uname -s)"

clean=false
usage=false
if [ $# -ge 1 ] && [ "$1" = "clean" ]; then
    clean=true
else
    while getopts ch flag
    do
      case "${flag}" in
        c) clean=true;;
        *) usage=true;;
      esac
done
fi

if [ "${usage}" = "true" ]; then
    echo "Usage: build [-c|-h]"
    echo " -c    clean build"
    echo " -h    help (this usage)"
    echo
    echo " * Run without any arguments to build cm-client"
    exit 0
fi

if [ "${clean}" = "true" ]; then
    # We clean CiscoSSL because its build is done in the source directory
    ./scripts/build_ciscossl.sh -c
    if [ -d debug ]; then
        rm -fr debug
    fi
    echo
    echo "** Build clean completed **"
else
    mkdir -p debug
    CMAKE_EXTRA_ARGS=""
    if [ "${SYSTEM}" == "Darwin" ]; then
        CMAKE_EXTRA_ARGS="-G Xcode"
    fi
    pushd debug
        cmake ${CMAKE_EXTRA_ARGS} ../ .
        cmake --build .
    popd

    echo
    echo "** Build completed **"
    echo
    echo " - build directory:	./debug"
    echo " - 3rd party exports:	./debug/export/{lib,include}"
    echo
    echo " Go to build directory, and run 'make' after making changes"
fi
