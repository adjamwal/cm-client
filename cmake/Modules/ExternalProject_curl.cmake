# curl external project build file
# Copyright Cisco Systems, Inc. 2022

include(ExternalProject)

set(component_name curl)
set(component_dst_dir "${CMAKE_CURRENT_BINARY_DIR}/third-party-${component_name}-prefix/src/third-party-${component_name}")
set(component_install_prefix "${CMAKE_CURRENT_SOURCE_DIR}/third-party/${component_name}/export")

if(NOT BUILD_ALL_THIRD_PARTY)
    download_component(${component_name} ${component_dst_dir})
endif()

set(CURL_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party/curl/src")

if(NOT TARGET "third-party-${component_name}")
    #
    # TODO Also support Linux toolchain
    #
    ExternalProject_Add(
        third-party-${component_name}
        SOURCE_DIR "${CURL_SRC_DIR}"
        # If needed we may want to add touch to ensure configure files and such are more recent
        # than the configure.ac files and such
        #COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_SOURCE_DIR}/third-party/curl/src/configure
        CONFIGURE_COMMAND "${CURL_SRC_DIR}/configure"
            CC=${XCODE_CC}
            CPP=${XCODE_CPP}
            CPPFLAGS=-I${CM_THIRDPARTY_EXPORT}/include
            LDFLAGS=-L${CM_THIRDPARTY_EXPORT}/lib
            --prefix=${component_install_prefix}
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

    add_custom_command(TARGET third-party-curl PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory "${component_install_prefix}")

    #
    # If needed we can use it to add touch commands to give build generated files a newer timestamp to avoid
    # regeneration of autotools build products
    #add_custom_command(TARGET third-party-curl PRE_BUILD COMMAND ${CMAKE_COMMAND} -E touch "${CMAKE_CURRENT_SOURCE_DIR}/third-party/curl/src/_autotool_generated_file_")

   upload_component(${component_name})
endif()

add_dependencies(third-party-curl third-party-ciscossl)
