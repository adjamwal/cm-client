# cmid external project build file
# Copyright Cisco Systems, Inc. 2022

include(ExternalProject)

set(component_name EndpointIdentity)
set(component_dst_dir "${CMAKE_CURRENT_BINARY_DIR}/third-party-${component_name}-prefix/src/third-party-${component_name}")
set(component_install_prefix "${CMAKE_CURRENT_SOURCE_DIR}/${component_name}/export")

if(NOT BUILD_ALL_THIRD_PARTY AND NOT BUILD_ENDPOINT_IDENTITY_THIRD_PARTY)
    download_component(${component_name} ${component_dst_dir})

    if(APPLE AND TARGET "third-party-${component_name}" AND DEFINED SIGNING_CERT)
        ExternalProject_Add_Step(
            third-party-${component_name}
            sign_binaries
            COMMENT "-- Signing binaries after Artifactory download"
            COMMAND /usr/bin/codesign -o runtime --verbose --force --deep --sign ${SIGNING_CERT} "${CM_THIRDPARTY_EXPORT}/bin/csc_cmid"
            COMMAND /usr/bin/codesign -o runtime --verbose --force --deep --sign ${SIGNING_CERT} "${CM_THIRDPARTY_EXPORT}/lib/libcmidapi.dylib"
            DEPENDEES install
        )
    endif()
endif()

if(NOT TARGET "third-party-${component_name}")
    #
    # TODO Be more specific about tool chain
    # TODO Adapt to make OSX options based on target
    # TODO Update CMAKE_COMMAND to take in actual version numbers
    # TODO Pass CM_SHARED_LOG_PATH on Debug builds if we want to log in the build path
    #        -DCM_SHARED_LOG_PATH=${CMAKE_CURRENT_BINARY_DIR}
    # TODO Currently project has no rules to install, add INSTALL_COMMAND (such as below)
    #      Right now, it's been forked and we added our own install directives
    #    INSTALL_COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/cmid/src/third-party-cmid-build/controlplugin/libcmidcontrolplugin.dylib ${CM_THIRDPARTY_EXPORT}/lib/.
    if(APPLE)
    ExternalProject_Add(
        third-party-${component_name}
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/EndpointIdentity/cmid
        PREFIX ${component_install_prefix}
        CMAKE_GENERATOR ${CMAKE_GENERATOR}
        CMAKE_ARGS
            -DBUILD_TESTING=0
            -DCONTROL_PLUGIN_STATIC_LIB=1
            -DCMAKE_BUILD_TYPE=RelWithDebInfo
            -DCMAKE_INSTALL_PREFIX:PATH=${component_install_prefix}
            -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES_}
            -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
        CMAKE_COMMAND VERSION=1.1 RELNUM=1111 ${CMAKE_COMMAND}
    )
    else()
    ExternalProject_Add(
        third-party-${component_name}
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/EndpointIdentity/cmid
        PREFIX ${component_install_prefix}
        DEPENDS third-party-libxml2
        CMAKE_GENERATOR ${CMAKE_GENERATOR}
        CMAKE_ARGS
            -DBUILD_TESTING=0
            -DCONTROL_PLUGIN_STATIC_LIB=1
            -DCMAKE_BUILD_TYPE=RelWithDebInfo
            -DCMAKE_INSTALL_PREFIX:PATH=${component_install_prefix}
            -DLIBXML2_LIBRARY=${LIBXML2_LIBRARY}
            -DLIBXML2_INCLUDE_DIR=${LIBXML2_INCLUDE_DIR}
            -DCM_THIRDPARTY_EXPORT=${CM_THIRDPARTY_EXPORT}
        CMAKE_COMMAND VERSION=1.1 RELNUM=1111 ${CMAKE_COMMAND}
    )
    endif()

    upload_component(${component_name} sign_dependee_step)

    if(APPLE AND DEFINED SIGNING_CERT)
        ExternalProject_Add_Step(
            third-party-${component_name}
            sign_binaries
            COMMENT "-- Signing binaries after build of EndpointIdentity"
            COMMAND /usr/bin/codesign -o runtime --verbose --force --deep --sign ${SIGNING_CERT} "${CM_THIRDPARTY_EXPORT}/bin/csc_cmid"
            COMMAND /usr/bin/codesign -o runtime --verbose --force --deep --sign ${SIGNING_CERT} "${CM_THIRDPARTY_EXPORT}/lib/libcmidapi.dylib"
            DEPENDEES ${sign_dependee_step}
        )
    endif()
endif()
