#!/bin/sh -e
# Copyright 2022 Cisco Systems, Inc.

SYSTEM="$(uname -s)"

clean=false
usage=false
if [ $# -ge 1 ] && [ "$1" = "clean" ]; then
    clean=true
else
    while getopts chx flag
    do
      case "${flag}" in
        c) clean=true;;
        x) xcode=true;; # Only applicable to macOS
        *) usage=true;;
      esac
done
fi

if [ "${usage}" = "true" ]; then
    echo "Usage: build [-c|-h]"
    echo " -c    clean build"
    echo " -h    help (this usage)"
    echo " -x    Xcode project generator (macOS only)"
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
    if [ "${SYSTEM}" = "Darwin" ] && [ "${xcode}" = "true" ]; then
        CMAKE_EXTRA_ARGS="-G Xcode"
        pushd debug
            cmake ${CMAKE_EXTRA_ARGS} ../ .
        popd

        #
        # The install step will succeed but the files may not be
        # copied over on the install step.
        # This means it will require the user to manually run the cmake step
        # for that specific third-party component
        #
        # Find the build directory and run:
        # % cmake --build . --config Debug --target install
        #
        # The build directory is usually of the form:
        # > ./debug/third-party/<third-party-component>/src/third-party-<third-party-component>-build

        echo
        echo "** Xcode build generated **"
        echo
        echo "** Open the project debug/cm-client.xcodeproj in Xcode to build **"
        echo
        echo " % open debug/cm-client.xcodeproj"
        echo
        echo " NOTE: CMake install targets may sometimes be unreliable and a manual run on the command"
        echo "       line may be necessary for some third-party components"
    else
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
fi
