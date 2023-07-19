# crashpad external project build file
# Copyright Cisco Systems, Inc. 2022

include(ExternalProject)

set(component_name crashpad)
set(component_dst_dir "${CMAKE_CURRENT_BINARY_DIR}/third-party-${component_name}-prefix/src/third-party-${component_name}")
set(component_src_dir "${CMAKE_CURRENT_SOURCE_DIR}/third-party/${component_name}/${component_name}")
set(component_install_prefix "${CMAKE_CURRENT_SOURCE_DIR}/third-party/${component_name}/export")

if(NOT BUILD_ALL_THIRD_PARTY AND NOT BUILD_CRASHPAD)
    download_component(${component_name} ${component_dst_dir})
endif()

set(CRASHPAD_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party/${component_name}")
set(CRASHPAD_BUILD_DIR "${CRASHPAD_SRC_DIR}/crashpad/out/Debug")
set(CRASHPAD_MINI_CHROMIUM "${CRASHPAD_SRC_DIR}/crashpad/third_party/mini_chromium")

if(NOT TARGET "third-party-${component_name}")
    #
    # TODO Also support Linux toolchain
    #
    ExternalProject_Add(
        third-party-${component_name}
        DEPENDS third-party-ciscossl third-party-curl
        SOURCE_DIR "${CRASHPAD_SRC_DIR}"
        CONFIGURE_COMMAND
            COMMAND echo "Running gclient sync on crashpad repo"
            COMMAND "${CRASHPAD_SRC_DIR}/build.sh" sync
        BUILD_COMMAND
            COMMAND echo "Building Crashpad..."
            COMMAND "${CRASHPAD_SRC_DIR}/build.sh" build "${CM_THIRDPARTY_EXPORT}"
        INSTALL_COMMAND
            COMMAND echo "Installing Crashpad..."
            COMMAND mkdir -p ${component_install_prefix}/lib
            COMMAND mv -f ${CRASHPAD_BUILD_DIR}/obj/client/libcommon.a ${component_install_prefix}/lib/libcrashpad_common.a
            COMMAND mv -f ${CRASHPAD_BUILD_DIR}/obj/third_party/mini_chromium/mini_chromium/base/libbase.a ${component_install_prefix}/lib/libcrashpad_base.a
            COMMAND mv -f ${CRASHPAD_BUILD_DIR}/obj/client/libclient.a ${component_install_prefix}/lib/libcrashpad_client.a
            COMMAND mv -f ${CRASHPAD_BUILD_DIR}/obj/util/libutil.a ${component_install_prefix}/lib/libcrashpad_util.a
            COMMAND mv -f ${CRASHPAD_BUILD_DIR}/obj/util/libnet.a ${component_install_prefix}/lib/libcrashpad_net.a
            COMMAND mv -f ${CRASHPAD_BUILD_DIR}/obj/minidump/libminidump.a ${component_install_prefix}/lib/libcrashpad_minidump.a
            COMMAND mv -f ${CRASHPAD_BUILD_DIR}/obj/handler/libhandler.a ${component_install_prefix}/lib/libcrashpad_handler_lib.a
            COMMAND mv -f ${CRASHPAD_BUILD_DIR}/obj/snapshot/libcontext.a ${component_install_prefix}/lib/libcrashpad_context.a
            COMMAND mv -f ${CRASHPAD_BUILD_DIR}/obj/snapshot/libsnapshot.a ${component_install_prefix}/lib/libcrashpad_snapshot.a
            COMMAND mkdir -p ${component_install_prefix}/bin
            COMMAND mv -f ${CRASHPAD_BUILD_DIR}/crashpad_handler ${component_install_prefix}/bin/crashpad_handler
            COMMAND mkdir -p ${component_install_prefix}/include/${component_name}/third_party
            COMMAND cp -rf ${CRASHPAD_MINI_CHROMIUM} ${component_install_prefix}/include/${component_name}/third_party
            # Remove .git as copying it causes permissions errors
            COMMAND rm -rf ${CRASHPAD_MINI_CHROMIUM}/mini_chromium/.git
    )

    add_custom_command(TARGET third-party-${component_name} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory "${component_install_prefix}")

    upload_component(${component_name} not_used)
endif()

# Copy over the crashpad for include directory, specify which ones to reduce bloat.
ExternalProject_Add_Step(third-party-${component_name} install_include
    WORKING_DIRECTORY ${component_src_dir}
    DEPENDEES install
    COMMAND
        mkdir -p ${CM_THIRDPARTY_EXPORT}/include/${component_name} &&
        cp -rf client ${CM_THIRDPARTY_EXPORT}/include/${component_name} &&
        cp -rf handler ${CM_THIRDPARTY_EXPORT}/include/${component_name} &&
        cp -rf minidump ${CM_THIRDPARTY_EXPORT}/include/${component_name} &&
        cp -rf snapshot ${CM_THIRDPARTY_EXPORT}/include/${component_name} &&
        cp -rf tools ${CM_THIRDPARTY_EXPORT}/include/${component_name} &&
        cp -rf util ${CM_THIRDPARTY_EXPORT}/include/${component_name} &&
        # Create symlinks for minichromium to build properly
        ln -sf ${CM_THIRDPARTY_EXPORT}/crashpad/third_party/mini_chromium/mini_chromium/base ${CM_THIRDPARTY_EXPORT}/include/crashpad/base &&
        ln -sf ${CM_THIRDPARTY_EXPORT}/crashpad/third_party/mini_chromium/mini_chromium/build ${CM_THIRDPARTY_EXPORT}/include/crashpad/build
)

add_dependencies(third-party-crashpad third-party-ciscossl third-party-curl)
