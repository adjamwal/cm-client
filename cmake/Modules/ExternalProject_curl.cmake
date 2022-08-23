# curl external project build file
# Copyright Cisco Systems, Inc. 2022

include(ExternalProject)

set(CISCOSSL_EXPORT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party/ciscossl/export")
set(XCODE_TOOLCHAIN_BIN "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin")
set(XCODE_MACOS_SDK "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk")
set(XCODE_CC "${XCODE_TOOLCHAIN_BIN}/clang -mmacosx-version-min=10.15 -isysroot ${XCODE_MACOS_SDK} -g -arch x86_64 -arch arm64")
set(XCODE_CPP "${XCODE_TOOLCHAIN_BIN}/clang -mmacosx-version-min=10.15 -isysroot ${XCODE_MACOS_SDK} -E")
set(XCODE_AR "${XCODE_TOOLCHAIN_BIN}/ar r")
set(XCODE_NM "${XCODE_TOOLCHAIN_BIN}/nm")
set(XCODE_RANLIB "${XCODE_TOOLCHAIN_BIN}/ranlib")

set(CURL_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party/curl/src")

#
# TODO Also support Linux toolchain
#
ExternalProject_Add(
    third-party-curl
    SOURCE_DIR "${CURL_SRC_DIR}"
    CONFIGURE_COMMAND "${CURL_SRC_DIR}/configure"
        CC=${XCODE_CC}
        CPP=${XCODE_CPP}
        CPPFLAGS=-I${CISCOSSL_EXPORT_DIR}/include
        LDFLAGS=-L${CISCOSSL_EXPORT_DIR}/lib
        --prefix=${CMAKE_CURRENT_SOURCE_DIR}/third-party/curl/export
        --enable-fts5=no
        --enable-json1=no
        --without-libidn2
        --without-libssh2
        --without-zstd
        --disable-ftp
        --disable-file
        --disable-ldap
        --disable-ldaps
        --disable-rtsp
        --disable-dict
        --disable-telnet
        --disable-tftp
        --disable-pop3
        --disable-imap
        --disable-smb
        --disable-smtp
        --disable-gopher
        --disable-mqtt
        --disable-shared
        --with-ssl=${CISCOSSL_EXPORT_DIR}
        --without-ca-bundle
        --without-ca-path
        curl_disallow_poll=yes
    BUILD_COMMAND make AR="${XCODE_AR}" NM="${XCODE_NM}" RANLIB="${XCODE_RANLIB}"
    INSTALL_COMMAND make install
)

add_custom_command(TARGET third-party-curl PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/third-party/curl/export")
