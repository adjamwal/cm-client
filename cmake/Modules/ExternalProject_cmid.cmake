# cmid external project build file
# Copyright Cisco Systems, Inc. 2022

include(ExternalProject)

#
# TODO Be more specific about tool chain
# TODO Adapt to make OSX options based on target
# TODO Update CMAKE_COMMAND to take in actual version numbers
# TODO Currently project has no rules to install, so INSTALL_COMMAND is missing below
#    cp ${CMAKE_CURRENT_BINARY_DIR}/cmid/src/third-party-cmid-build/agent/csc_cmid ${CM_THIRDPARTY_EXPORT}/bin/.
#    cp ${CMAKE_CURRENT_SOURCE_DIR}/EndpointIdentity/cmid/controlplugin/CMIDAgentController.h ${CM_THIRDPARTY_EXPORT}/include/.
ExternalProject_Add(
    third-party-cmid
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/EndpointIdentity/cmid
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/cmid
    CMAKE_ARGS
        -DBUILD_TESTING=0
        -DCMAKE_BUILD_TYPE=RelWithDebInfo
        -DCMAKE_INSTALL_PREFIX:PATH=${CM_THIRDPARTY_EXPORT}
        -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES_}
        -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
    CMAKE_COMMAND VERSION=1.1 RELNUM=1111 ${CMAKE_COMMAND}
    INSTALL_COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/cmid/src/third-party-cmid-build/controlplugin/libcmidcontrolplugin.dylib ${CM_THIRDPARTY_EXPORT}/lib/.
)
