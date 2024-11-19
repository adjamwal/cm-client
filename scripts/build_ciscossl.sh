#!/bin/bash -e
# This script builds the CiscoSSL and installs to a specified
# third-party/ciscossl export directory.
# Use the -f flag with this script to build and link FOM
# Use the -c flag with this script to do a clean
# for CiscoSSL

SYSTEM="$(uname -s)"
fom=false
clean=false

BUILD_ARCHS=(
    "x86_64"
    "arm64"
)

while getopts fc flag
do
  case "${flag}" in
    f) fom=true;;
    c) clean=true;;
  esac
done

if [ ! -d "client" ] || [ ! -d "third-party" ] || [ ! -d "cmake" ]; then
    echo "ERROR: This script must be run from the cm-client root directory!"
    exit 1
fi

if [ -z "${WORKSPACE_ROOT}" ]; then
    WORKSPACE_ROOT="$(pwd)"
    # WORKSPACE_ROOT should now contain the full directory path
    echo "Using WORKSPACE_ROOT=${WORKSPACE_ROOT}"
fi

CISCOSSL_EXPORT_DIR="${WORKSPACE_ROOT}/third-party/ciscossl/export"

build_ciscossl_forarch()
{
    local EXPORT_DIR=${CISCOSSL_EXPORT_DIR}

    # Use Xcode compiler and archiver with OS X 10.15 SDK.
    if [ "${SYSTEM}" == "Darwin" ]; then
        local ARCH=$1
        local XCODE_BIN_ROOT='/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin'
        local SDK_ROOT='/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk'
        local SDK_CC="${XCODE_BIN_ROOT}/clang -mmacosx-version-min=10.15 -isysroot ${SDK_ROOT}"
        local SDK_AR="${XCODE_BIN_ROOT}/ar"
        local SDK_NM="${XCODE_BIN_ROOT}/nm"
        local SDK_RANLIB="${XCODE_BIN_ROOT}/ranlib"
        local CURR_ARCH="darwin64-${ARCH}-cc"

        # Append arch to EXPORT_DIR
        EXPORT_DIR=${EXPORT_DIR}-${ARCH}

    # Linux
    elif [ "${SYSTEM}" == "Linux" ]; then
        local BIN_ROOT='/usr/bin'
        local SDK_CC="${BIN_ROOT}/gcc"
        local SDK_AR="${BIN_ROOT}/ar"
        local SDK_NM="${BIN_ROOT}/nm"
        local SDK_RANLIB="${BIN_ROOT}/ranlib"
        local CURR_ARCH="linux-$(arch)"

    else
        echo "Unsupported system! Aborting"
        exit 1
    fi

    rm -fr /tmp/fips
    local CONFIGURE_FOM_FLAGS="${CURR_ARCH} fipscanisteronly --prefix=/tmp/fips"

    # Build FOM
    if [ "$fom" = true ]; then
      FOM_TARBALL=""
      pushd "${WORKSPACE_ROOT}/third-party/ciscossl/src/fom"
          # Will only be one file tar ball in that directory
          FOM_TARBALL=$(ls -1 *.tar.gz | head -1)
      popd
      if [ -z "${FOM_TARBALL}" ]; then
          echo "ERROR: Could not find FOM tarball in: ${WORKSPACE_ROOT}/third-party/ciscossl/src/fom"
          exit 1
      fi
      echo "Building FOM ${FOM_TARBALL}..."
      rm -fr /tmp/fom_tmp
      mkdir /tmp/fom_tmp
      pushd /tmp/fom_tmp
          tar xvfz "${WORKSPACE_ROOT}/third-party/ciscossl/src/fom/${FOM_TARBALL}"
          OUTPUT_DIR_NAME="${FOM_TARBALL%%.tar.gz}"
          pushd "${OUTPUT_DIR_NAME}"
              ./Configure ${CONFIGURE_FOM_FLAGS}
               make
               make install
          popd
      popd
    fi

    if [ "$fom" = "true" ]; then
        local CONFIGURE_FLAGS="${CURR_ARCH} fips --with-fipsdir=/tmp/fips -shared -g --prefix=${EXPORT_DIR}"
    else
        local CONFIGURE_FLAGS="${CURR_ARCH} -shared -g --prefix=${EXPORT_DIR}"
    fi

    # Build CiscoSSL shared libaries.  Enable debug symbols.
    pushd "${WORKSPACE_ROOT}/third-party/ciscossl/src"
        \rm -rf "${EXPORT_DIR}"
        echo "Configuring CiscoSSL with options ${CONFIGURE_FLAGS}"
        ./Configure ${CONFIGURE_FLAGS}
        make clean # Need to do this to ensure we don't link against the wrong architecture
        make depend
        make CC="${SDK_CC}" AR="${SDK_AR}" NM="${SDK_NM}" RANLIB="${SDK_RANLIB}"
        #make test
        make install
    popd
}

build_ciscossl_mac()
{
    rm -rf "${CISCOSSL_EXPORT_DIR}"

    for i in "${BUILD_ARCHS[@]}"; do
        build_ciscossl_forarch "${i}"
    done

    mkdir "${CISCOSSL_EXPORT_DIR}"
    # The arm64 and x86_64 export headers should be identical.
    cp -a "${CISCOSSL_EXPORT_DIR}-${BUILD_ARCHS[0]}/include" "${CISCOSSL_EXPORT_DIR}"

    # Leave static libaries in the lib directory so it can be referenced for
    # static linking. Place shared libraries in a separate directory that can be
    # referenced for dynamic linking.
    mkdir "${CISCOSSL_EXPORT_DIR}/lib"
    local ARCHS_PATTERN="@("
    for i in "${BUILD_ARCHS[@]}"; do
        ARCHS_PATTERN="${ARCHS_PATTERN}${i}|"
    done
    # Remove trailing |
    ARCHS_PATTERN="${ARCHS_PATTERN%?})"

    shopt -s extglob
    lipo -create \
      "${CISCOSSL_EXPORT_DIR}-"${ARCHS_PATTERN}"/lib/libcrypto.a" \
      -output "${CISCOSSL_EXPORT_DIR}/lib/libcrypto.a"
    lipo -create \
      "${CISCOSSL_EXPORT_DIR}-"${ARCHS_PATTERN}"/lib/libssl.a" \
      -output "${CISCOSSL_EXPORT_DIR}/lib/libssl.a"
    mkdir "${CISCOSSL_EXPORT_DIR}/libshared"
    lipo -create \
      "${CISCOSSL_EXPORT_DIR}-"${ARCHS_PATTERN}"/lib/libcrypto.1.1.dylib" \
      -output "${CISCOSSL_EXPORT_DIR}/libshared/libcrypto.1.1.dylib"
    ln -s "${CISCOSSL_EXPORT_DIR}/libshared/libcrypto.1.1.dylib" \
      "${CISCOSSL_EXPORT_DIR}/libshared/libcrypto.dylib"
    lipo -create \
      "${CISCOSSL_EXPORT_DIR}-"${ARCHS_PATTERN}"/lib/libssl.1.1.dylib" \
      -output "${CISCOSSL_EXPORT_DIR}/libshared/libssl.1.1.dylib"
    ln -s "${CISCOSSL_EXPORT_DIR}/libshared/libssl.1.1.dylib" \
      "${CISCOSSL_EXPORT_DIR}/libshared/libssl.dylib"

    # Clean up
    for i in "${BUILD_ARCHS[@]}"; do
        rm -fr "${CISCOSSL_EXPORT_DIR}-${i}"
    done
}

build_ciscossl_linux()
{
    # Call with no arguments, on Linux we only build for x86_64
    build_ciscossl_forarch
}

build_ciscossl()
{
    if [ "${SYSTEM}" == "Darwin" ]; then
        build_ciscossl_mac
    else
        build_ciscossl_linux
    fi
}

if [ "$clean" = "true" ]; then
    rm -rf "${CISCOSSL_EXPORT_DIR}"
    pushd "${WORKSPACE_ROOT}/third-party/ciscossl/src"
        if [ -f Makefile ]; then
            make clean
        fi
    popd
    if [ "${SYSTEM}" == "Darwin" ]; then
        ARCHS_PATTERN="@("
        for i in "${BUILD_ARCHS[@]}"; do
            ARCHS_PATTERN="${ARCHS_PATTERN}${i}|"
        done
        # Remove trailing |
        ARCHS_PATTERN="${ARCHS_PATTERN%?})"
        rm -fr "${CISCOSSL_EXPORT_DIR}-${ARCHS_PATTERN}"
    fi
elif [ ! -d "${CISCOSSL_EXPORT_DIR}/include/openssl" ]; then
    build_ciscossl
else
    echo
    echo "** ====================================================="
    echo "** CiscoSSL already built, do a clean to force a rebuild"
    echo "**"
    echo
fi
