# CMakeLists.txt
# Copyright 2022, Cisco Systems, Inc.

include(TargetEnableWarnings)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CM_KEY "uc")

set(CM_CLIENT_SRCS
    Daemon.cpp
    Daemon.hpp
    main.cpp
    ComponentLoader/CMIDLoader.hpp
    ComponentLoader/CMIDLoader.cpp
    ComponentLoader/PMLoader.hpp
    ComponentLoader/PMLoader.cpp
    Logger/CMLogger.cpp
    $<$<BOOL:${APPLE}>:${cm-client_SOURCE_DIR}/common/macOS/FileWatcher.mm>
    $<$<BOOL:${APPLE}>:${cm-client_SOURCE_DIR}/common/macOS/FileWatcher.hpp>
    $<$<BOOL:${APPLE}>:${cm-client_SOURCE_DIR}/common/macOS/ProxyWatcher.cpp>
    $<$<BOOL:${APPLE}>:${cm-client_SOURCE_DIR}/common/macOS/ProxyWatcher.hpp>
    $<$<BOOL:${APPLE}>:crashpad/CrashpadTuner.h>
    $<$<BOOL:${APPLE}>:crashpad/CrashpadTuner.cpp>
)

if(LINUX)
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/Logger/CMLogger.cpp PROPERTIES COMPILE_FLAGS "-Wno-maybe-uninitialized")

endif()

add_executable(csccloudmanagement ${CM_CLIENT_SRCS})

add_dependencies(csccloudmanagement
    third-party-ciscossl
    third-party-curl
    third-party-jsoncpp
    third-party-EndpointIdentity
    third-party-PackageManager
    third-party-spdlog
)

if(APPLE)
add_dependencies(csccloudmanagement
    third-party-crashpad
    third-party-ProxyDiscovery-Mac
)
endif()

target_include_directories(csccloudmanagement
    PUBLIC
    ${cm-client_SOURCE_DIR}
    ${CM_THIRDPARTY_EXPORT}/include
    $<$<BOOL:${APPLE}>:${CM_THIRDPARTY_EXPORT}/include/crashpad>
    $<$<BOOL:${APPLE}>:${CM_THIRDPARTY_EXPORT}/include/crashpad/third_party/mini_chromium/mini_chromium>
    $<$<BOOL:${APPLE}>:${CM_THIRDPARTY_EXPORT}/include/ProxyDiscovery>
    ${cm-client_SOURCE_DIR}/util
)

target_link_directories(csccloudmanagement BEFORE
    PRIVATE
    ${CM_THIRDPARTY_EXPORT}/lib
)

target_link_libraries(csccloudmanagement
    PUBLIC
    cmidapi
    configshared
    cmidcontrolplugin
    common
    pmcontrolplugin
    $<$<BOOL:${APPLE}>:ProxyDiscovery>
    jsoncpp
    $<$<BOOL:${APPLE}>:crashpad_client>
    $<$<BOOL:${APPLE}>:crashpad_common>
    $<$<BOOL:${APPLE}>:crashpad_context>
    $<$<BOOL:${APPLE}>:crashpad_handler_lib>
    $<$<BOOL:${APPLE}>:crashpad_minidump>
    $<$<BOOL:${APPLE}>:crashpad_net>
    $<$<BOOL:${APPLE}>:crashpad_snapshot>
    $<$<BOOL:${APPLE}>:crashpad_util>
    $<$<BOOL:${APPLE}>:mig_output>
    $<$<BOOL:${APPLE}>:crashpad_base>
    $<$<BOOL:${APPLE}>:bsm>
    util
    pthread
    $<$<BOOL:${LINUX}>:stdc++fs>
    "$<$<BOOL:${APPLE}>:-framework SystemConfiguration>"
    "$<$<BOOL:${APPLE}>:-framework IOKit>"
    "$<$<BOOL:${APPLE}>:-framework CoreServices>"
    "$<$<BOOL:${APPLE}>:-framework CoreGraphics>"
    "$<$<BOOL:${APPLE}>:-framework CoreText>"
    "$<$<BOOL:${APPLE}>:-framework Security>"
    "$<$<BOOL:${APPLE}>:-framework Foundation>"
)

if(LINUX)
set_target_properties(csccloudmanagement
PROPERTIES
    INSTALL_RPATH "$ORIGIN/../lib"
    INSTALL_RPATH_USE_LINK_PATH TRUE
)
endif()

target_compile_definitions(csccloudmanagement
    PUBLIC
    "$<$<CONFIG:DEBUG>:DEBUG>"
    "$<$<CONFIG:DEBUG>:CMID_DAEMON_PATH=\"${CM_THIRDPARTY_EXPORT}/bin\">"
    "$<$<CONFIG:DEBUG>:CM_CONFIG_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/config\">"
    "$<$<CONFIG:DEBUG>:CM_SHARED_LOG_PATH=\"${CMAKE_CURRENT_BINARY_DIR}/\">"
    "$<$<CONFIG:DEBUG>:CMID_LOG_PATH=\"/Library/Logs/Cisco/SecureClient/CloudManagement/\">"
)

if(CMAKE_GENERATOR STREQUAL "Xcode")
    macro(set_xcode_property TARGET XCODE_PROPERTY XCODE_VALUE)
    set_property (TARGET ${TARGET} PROPERTY XCODE_ATTRIBUTE_${XCODE_PROPERTY}
        ${XCODE_VALUE})
    endmacro (set_xcode_property)

    set_xcode_property(csccloudmanagement CLANG_CXX_LANGUAGE_STANDARD "gnu++17")
    set_xcode_property(csccloudmanagement CLANG_CXX_LIBRARY "libc++")

    # For code signing
    if(DEFINED SIGNING_CERT)
        set_xcode_property(csccloudmanagement CODE_SIGN_IDENTITY "${SIGNING_CERT}")
    endif()
    # Other signing attributes of interest
    # CODE_SIGN_ENTITLEMENTS my.entitlements
    # PRODUCT_NAME "My Product"
    # DEVELOPMENT_TEAM ${DEVELOPMENT_TEAM_ID}
    # PROVISIONING_PROFILE_SPECIFIER my_profile_name
endif()

target_enable_warnings(csccloudmanagement)

install(TARGETS csccloudmanagement 
        DESTINATION ${CM_THIRDPARTY_EXPORT}/bin/
        COMPONENT installstaging)

# Include tests
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

