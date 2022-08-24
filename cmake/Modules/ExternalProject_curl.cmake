# curl external project build file
# Copyright Cisco Systems, Inc. 2022

include(ExternalProject)

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
    # If needed we may want to add touch to ensure configure files and such are more recent
    # than the configure.ac files and such
    #COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_SOURCE_DIR}/third-party/curl/src/configure
    CONFIGURE_COMMAND "${CURL_SRC_DIR}/configure"
        CC=${XCODE_CC}
        CPP=${XCODE_CPP}
        CPPFLAGS=-I${CM_THIRDPARTY_EXPORT}/include
        LDFLAGS=-L${CM_THIRDPARTY_EXPORT}/lib
        --prefix=${CM_THIRDPARTY_EXPORT}
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
        --with-ssl=${CM_THIRDPARTY_EXPORT}
        --without-ca-bundle
        --without-ca-path
        curl_disallow_poll=yes
    BUILD_COMMAND make AR="${XCODE_AR}" NM="${XCODE_NM}" RANLIB="${XCODE_RANLIB}"
    INSTALL_COMMAND make install
)

add_custom_command(TARGET third-party-curl PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/third-party/curl/export")
#
# If needed we can use it to add touch commands to give build generated files a newer timestamp to avoid
# regeneration of autotools build products
#add_custom_command(TARGET third-party-curl PRE_BUILD COMMAND ${CMAKE_COMMAND} -E touch "${CMAKE_CURRENT_SOURCE_DIR}/third-party/curl/src/_autotool_generated_file_")
