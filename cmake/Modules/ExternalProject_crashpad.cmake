# crashpad external project build file
# Copyright Cisco Systems, Inc. 2022

include(ExternalProject)

set(component_name crashpad)
set(component_dst_dir "${CMAKE_CURRENT_BINARY_DIR}/third-party-${component_name}-prefix/src/third-party-${component_name}")
set(component_src_dir "${CMAKE_CURRENT_SOURCE_DIR}/third-party/${component_name}/${component_name}")
if(CMAKE_GENERATOR STREQUAL Xcode)
    set(component_build_folder_prefix "xcode_")
else()
    set(component_build_folder_prefix "")
endif()
set(component_install_prefix "${CMAKE_CURRENT_SOURCE_DIR}/third-party/${component_name}/${component_build_folder_prefix}export")

if(NOT BUILD_ALL_THIRD_PARTY AND NOT BUILD_CRASHPAD)
    download_component(${component_name} ${component_dst_dir})
endif()

set(CRASHPAD_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party/${component_name}")
set(CRASHPAD_BUILD_DIR "${CRASHPAD_SRC_DIR}/crashpad/out/Debug")
set(CRASHPAD_MINI_CHROMIUM "${CRASHPAD_SRC_DIR}/crashpad/third_party/mini_chromium")

if(NOT TARGET "third-party-${component_name}")
    if(APPLE)
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
                COMMAND mv -f ${CRASHPAD_BUILD_DIR}/obj/client/libclient.a ${component_install_prefix}/lib/libcrashpad_client.a
                COMMAND mv -f ${CRASHPAD_BUILD_DIR}/obj/util/libutil.a ${component_install_prefix}/lib/libcrashpad_util.a
                COMMAND mv -f ${CRASHPAD_BUILD_DIR}/obj/util/libnet.a ${component_install_prefix}/lib/libcrashpad_net.a
                COMMAND mv -f ${CRASHPAD_BUILD_DIR}/obj/minidump/libminidump.a ${component_install_prefix}/lib/libcrashpad_minidump.a
                COMMAND mv -f ${CRASHPAD_BUILD_DIR}/obj/handler/libhandler.a ${component_install_prefix}/lib/libcrashpad_handler_lib.a
                COMMAND mv -f ${CRASHPAD_BUILD_DIR}/obj/snapshot/libcontext.a ${component_install_prefix}/lib/libcrashpad_context.a
                COMMAND mv -f ${CRASHPAD_BUILD_DIR}/obj/snapshot/libsnapshot.a ${component_install_prefix}/lib/libcrashpad_snapshot.a
                COMMAND mv -f ${CRASHPAD_BUILD_DIR}/obj/util/libmig_output.a ${component_install_prefix}/lib/libmig_output.a
                COMMAND mv -f ${CRASHPAD_BUILD_DIR}/obj/third_party/mini_chromium/mini_chromium/base/libbase.a ${component_install_prefix}/lib/libcrashpad_base.a
                COMMAND mkdir -p ${component_install_prefix}/bin
                COMMAND mv -f ${CRASHPAD_BUILD_DIR}/crashpad_handler ${component_install_prefix}/bin/cmreport_handler
                COMMAND mv -f ${CRASHPAD_BUILD_DIR}/crashpad_database_util ${component_install_prefix}/bin/crashpad_database_util
                COMMAND mkdir -p ${component_install_prefix}/include/${component_name}/third_party
                # Remove .git as copying it causes permissions errors
                COMMAND rm -rf ${CRASHPAD_MINI_CHROMIUM}/mini_chromium/.git
                COMMAND cp -rf ${CRASHPAD_MINI_CHROMIUM} ${component_install_prefix}/include/${component_name}/third_party

        )

        add_custom_command(TARGET third-party-${component_name} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory "${component_install_prefix}")
    elseif(LINUX)
        set(depot_tools_dir ${CMAKE_CURRENT_BINARY_DIR}/depot_tools/src/depot_tools)
        set(depot_tools_python_path ${depot_tools_dir}/python-bin)
        set(depot_tools_env PATH=${depot_tools_dir}:${depot_tools_python_path}:$ENV{PATH})

        set(gclient_command gclient)
        
        if(NOT EXISTS ${depot_tools_dir})
            ExternalProject_Add(
                depot_tools
                PREFIX depot_tools
                GIT_REPOSITORY https://chromium.googlesource.com/chromium/tools/depot_tools.git
                GIT_TAG main
                PATCH_COMMAND ${depot_tools_patch_command}
                CONFIGURE_COMMAND ""
                BUILD_COMMAND ""
                INSTALL_COMMAND ""
            )
        else()
            add_custom_target(depot_tools)
        endif()

        set(include_dir "${CM_THIRDPARTY_EXPORT}/include")
        set(lib_dir "${CM_THIRDPARTY_EXPORT}/lib")
        set(third-party-${component_name}_CFLAGS "-g -fPIC -fstack-protector-all -D_FORTIFY_SOURCE=2 -D__STDC_FORMAT_MACROS -I${include_dir} -Wno-deprecated-declarations")
        set(third-party-${component_name}_LDFLAGS "-ldl -L${lib_dir}")
 
        set(gn_env ${depot_tools_env})

        set(gn_args "\
            crashpad_use_curl_openssl_for_http = true \
            extra_cflags_cc = \"${third-party-${component_name}_CFLAGS}\" \
            extra_ldflags = \"${third-party-${component_name}_LDFLAGS}\" \
            ${gn_clang_path} \
        ")

        set(mini_chromium_install_include_prefix "${component_install_prefix}/include/crashpad/third_party/mini_chromium")
        set(mini_chromium_install_include_dir "${mini_chromium_install_include_prefix}/mini_chromium")

        ExternalProject_Add(
            third-party-${component_name}
            PREFIX third-party-${component_name}
            DEPENDS third-party-ciscossl third-party-curl depot_tools
            SOURCE_DIR ${CRASHPAD_SRC_DIR}
            CONFIGURE_COMMAND
                COMMAND cd ${CRASHPAD_SRC_DIR} && ${depot_tools_env} ${gclient_command} sync --nohooks
                COMMAND cd ${component_src_dir} && ${gn_env} ${PROJECT_SOURCE_DIR}/cmake/Modules/helper/configure_crashpad_helper.sh ${CRASHPAD_BUILD_DIR} ${gn_args}
            BUILD_COMMAND
                COMMAND ninja -C ${CRASHPAD_BUILD_DIR}
            INSTALL_COMMAND
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
                # Add chromium headers to include
                COMMAND mkdir -p ${mini_chromium_install_include_prefix}
                COMMAND cp -rf ${CRASHPAD_MINI_CHROMIUM}/mini_chromium ${mini_chromium_install_include_prefix}
                COMMAND cp -rf ${CRASHPAD_BUILD_DIR}/gen/build ${mini_chromium_install_include_dir}
                # Remove .git as copying it causes permissions errors
                COMMAND rm -rf ${mini_chromium_install_include_dir}/.git
            BUILD_BYPRODUCTS ${component_byproducts}
        )
    endif()
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
