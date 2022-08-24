#!/bin/sh -e
# Copyright 2022 Cisco Systems, Inc.

clean=false
usage=false
while getopts ch flag
do
  case "${flag}" in
    c) clean=true;;
    h) usage=true;;
  esac
done

if [ "${usage}" = "true" ]; then
    echo "Usage: build [-c|-h]"
    echo " -c    clean build"
    echo " -h    help (this usage)"
    echo
    echo " * Run without any arguments to build cm-client"
    exit 0
fi

if [ "${clean}" = "true" ]; then
    ./scripts/build_ciscossl.sh -c
    if [ -d debug ]; then
        rm -fr debug
    fi
    echo
    echo "** Build clean completed **"
else
    # Build CiscoSSL first
    ./scripts/build_ciscossl.sh

    # TODO Remove other third-party export directories

    mkdir -p debug
    pushd debug
        cmake ../ .
        cmake --build .
    popd

    echo
    echo "** Build completed **"
    echo
    echo " - build directory: debug"
    echo
    echo " Go to build directory, and run 'make' after making changes"
fi
