#!/bin/sh -e
# Copyright 2022 Cisco Systems, Inc.

SYSTEM="$(uname -s)"
clean=false
usage=false
if [ $# -ge 1 ] && [ "$1" = "clean" ]; then
    clean=true
else
    while getopts chrxs: flag
    do
      case "${flag}" in
        c) clean=true;;
        x) xcode=true;; # Only applicable to macOS
        r) release=true;;
        s) sign=true
           # Use - to let script choose the signing certificate
           if [ "${OPTARG}" != "-" ]; then
               echo "**Using signing certificate ${OPTARG}"
               SIGNING_CERT=${OPTARG}
           else
               echo "**No certificate specified, will find suitable certificate in Keychain"
           fi
           ;; # Only applicable to macOS
        *) usage=true;;
      esac
done
fi

if [ "${usage}" = "true" ] || [ "x$CM_BUILD_VER" = "x" ]; then
    if [ "x$CM_BUILD_VER" = "x" ]; then
        echo "***"
        echo "*** ERROR: CM_BUILD_VER is not set"
        echo
    fi
    echo "Usage: build [-c|-h]"
    echo " -c		clean build"
    echo " -h		help (this usage)"
    echo " -r		release (default: debug)"
    echo " -x		Xcode project generator (macOS only)"
    echo " -s <cert>	Sign binaries (use - for <cert> for build to select automatically)"
    echo
    echo " * Run without any arguments to build cm-client"
    echo
    echo "Required Environment Variables:"
    echo
    echo " CM_BUILD_VER                     Build version number (i.e. 1.0.0.0)"
    echo
    echo "Influential Environment Variables:"
    echo
    echo " BUILD_SUBMODULES_FROM_SRC        Builds all third-party, and submodules from source"
    echo " RETAIN_SYMBOLS                   Set to "yes" to retain symbols when building a release build"
    echo " DEV_ID_APP_CERT                  Developer ID Application Certificate"
    echo " DEV_ID_INSTALL_CERT              Developer ID Installer Certificate"
    echo
    echo "Examples:"
    echo
    echo " # Use Xcode, and find *developer* signing certificate in Keychain"
    echo " % CM_BUILD_VER=1.0.0 ./build -x -s -"
    echo
    echo " # Create a Makefile build signing using specified certificate in Keychain"
    echo " % CM_BUILD_VER=1.0.0 ./build -s \"Apple Development: John Smith (F34ACD4F4E)\""
    exit 0
fi

CMAKE_EXTRA_ARGS="-DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=1"
CMAKE_BUILD_DIR="debug"
BUILD_STAGING_DIR="$(pwd)/Staging"
if [ "${release}" = "true"  ]; then
    CMAKE_EXTRA_ARGS="-DCMAKE_BUILD_TYPE=RelWithDebInfo"
    CMAKE_BUILD_DIR="release"
fi
if [ "${BUILD_SUBMODULES_FROM_SRC}" = "YES" ]; then
    CMAKE_EXTRA_ARGS="-DBUILD_ALL_THIRD_PARTY:BOOL=ON ${CMAKE_EXTRA_ARGS}"
fi

if [ "${sign}" = "true" ]; then
    # If SIGNING_CERT is not defined, try to find appropriate signing certificate
    if [ -z "${SIGNING_CERT}" ]; then
        sign_cert_count=$(security find-identity -p codesigning -v | grep "Apple Development" | wc -l | xargs)
        echo "**Found ${sign_cert_count} matching certificate(s) in Keychain"
        if [ ${sign_cert_count} -eq 0 ]; then
            echo "ERROR: No certificate found in Keychain for codesigning"
            exit 1
        elif [ ${sign_cert_count} -eq 1 ]; then
            # If there's only one match, then we can just use "Apple Development" and let
            # codesign choose
            SIGNING_CERT="Apple Development"
        else
            # In the case where more than one signing certificate is available
            # we just choose the last match (assuming last is newest)
            SIGNING_CERT=$(security find-identity -p codesigning -v | grep "Apple Development" | cut -f 2 -d '"' | tail -1)
        fi
    fi
fi

if [ "${clean}" = "true" ]; then
    # We clean CiscoSSL because its build is done in the source directory
    ./scripts/build_ciscossl.sh -c
    if [ -d debug ]; then
        rm -fr debug
    fi
    if [ -d release ]; then
        rm -fr release
    fi
    echo
    echo "** Build clean completed **"
else
    mkdir -p "${CMAKE_BUILD_DIR}"
    if [ "${SYSTEM}" = "Darwin" ] && [ "${xcode}" = "true" ]; then
        CMAKE_EXTRA_ARGS="-G Xcode ${CMAKE_EXTRA_ARGS}"
        pushd "${CMAKE_BUILD_DIR}"
            if [ "${sign}" = "true" ]; then
                cmake ${CMAKE_EXTRA_ARGS} -DSIGNING_CERT="${SIGNING_CERT}" ../
            else
                cmake ${CMAKE_EXTRA_ARGS} ../
            fi
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
        # > ./${CMAKE_BUILD_DIR}/third-party/<third-party-component>/src/third-party-<third-party-component>-build

        echo
        echo "** Xcode build generated **"
        echo
        echo "** Open the project ${CMAKE_BUILD_DIR}/cm-client.xcodeproj in Xcode to build **"
        echo
        echo " % open ${CMAKE_BUILD_DIR}/cm-client.xcodeproj"
        echo
        echo " NOTE: CMake install targets may sometimes be unreliable and a manual run on the command"
        echo "       line may be necessary for some third-party components"
    else
        pushd "${CMAKE_BUILD_DIR}"
            if [ "${sign}" = "true" ]; then
                cmake ${CMAKE_EXTRA_ARGS} -DSIGNING_CERT="${SIGNING_CERT}" ../
            else
                cmake ${CMAKE_EXTRA_ARGS} ../
            fi
            cmake --build .
        popd

        echo
        echo "** Build completed **"
        echo
        echo " - build directory:	./${CMAKE_BUILD_DIR}"
        echo " - 3rd party exports:	./${CMAKE_BUILD_DIR}/export/{lib,include}"
        echo
        
        echo "** Running 'make' in ${CMAKE_BUILD_DIR} to build CM"
        pushd "${CMAKE_BUILD_DIR}"
            make
            # Copy files to debug/export/{bin,lib,...} directory for use by installer
            make install
        popd
        
        echo "** Building CM Installer **"
        BUILD_TYPE="${CMAKE_BUILD_DIR}"
        DMG_INSTALLER_DIR="Installer"
        DMG_BUILDER_SCRIPT="build_cm_installer.sh"
        pushd "${DMG_INSTALLER_DIR}"
            "./${DMG_BUILDER_SCRIPT}" "${BUILD_TYPE}" "${BUILD_STAGING_DIR}"
            echo "** CM Installer built successfully **"
        popd
    fi
fi
