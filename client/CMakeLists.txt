# CMakeLists.txt
# Copyright 2022, Cisco Systems, Inc.

include(TargetEnableWarnings)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CM_KEY "uc")

if(LINUX)
    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/Logger/CMLogger.cpp PROPERTIES COMPILE_FLAGS "-Wno-maybe-uninitialized")
endif()

set(component_name "csccloudmanagement")

add_executable(${component_name} 
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
    crashpad/CrashpadTuner.h
    crashpad/CrashpadTuner.cpp
    $<$<BOOL:${LINUX}>:${cm-client_SOURCE_DIR}/common/linux/ProxyWatcher.cpp>
    $<$<BOOL:${LINUX}>:${cm-client_SOURCE_DIR}/common/linux/ProxyWatcher.hpp>
)


add_dependencies(${component_name}
    third-party-ciscossl
    third-party-curl
    third-party-jsoncpp
    third-party-EndpointIdentity
    third-party-PackageManager
    third-party-spdlog
    third-party-crashpad
    third-party-ProxyDiscovery-Mac
)

target_include_directories(${component_name}
    PUBLIC
    ${cm-client_SOURCE_DIR}
    ${CM_THIRDPARTY_EXPORT}/include
    ${CM_THIRDPARTY_EXPORT}/include/crashpad
    ${CM_THIRDPARTY_EXPORT}/include/crashpad/third_party/mini_chromium/mini_chromium
    ${CM_THIRDPARTY_EXPORT}/include/ProxyDiscovery
    ${cm-client_SOURCE_DIR}/util
)

target_link_directories(${component_name} BEFORE
    PRIVATE
    ${CM_THIRDPARTY_EXPORT}/lib
)

target_link_libraries(${component_name}
    PUBLIC
    cmidapi
    configshared
    cmidcontrolplugin
    common
    pmcontrolplugin
    ProxyDiscovery
    jsoncpp
    crashpad_client
    crashpad_common
    crashpad_context
    crashpad_handler_lib
    crashpad_minidump
    crashpad_net
    crashpad_snapshot
    crashpad_util
    $<$<BOOL:${APPLE}>:mig_output>
    crashpad_base
    $<$<BOOL:${APPLE}>:bsm>
    util
    pthread
    $<$<BOOL:${LINUX}>:stdc++fs>
    $<$<BOOL:${LINUX}>:curl>
    $<$<BOOL:${LINUX}>:ssl>
    $<$<BOOL:${LINUX}>:crypto>
    $<$<BOOL:${LINUX}>:z>
    $<$<BOOL:${LINUX}>:dl>
    "$<$<BOOL:${APPLE}>:-framework SystemConfiguration>"
    "$<$<BOOL:${APPLE}>:-framework IOKit>"
    "$<$<BOOL:${APPLE}>:-framework CoreServices>"
    "$<$<BOOL:${APPLE}>:-framework CoreGraphics>"
    "$<$<BOOL:${APPLE}>:-framework CoreText>"
    "$<$<BOOL:${APPLE}>:-framework Security>"
    "$<$<BOOL:${APPLE}>:-framework Foundation>"
)

if(LINUX)
    set_target_properties(${component_name}
        PROPERTIES
            INSTALL_RPATH "$ORIGIN/../lib"
            INSTALL_RPATH_USE_LINK_PATH TRUE
    )
endif()

target_compile_definitions(${component_name}
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

    set_xcode_property(${component_name} CLANG_CXX_LANGUAGE_STANDARD "gnu++17")
    set_xcode_property(${component_name} CLANG_CXX_LIBRARY "libc++")

    # For code signing
    if(DEFINED SIGNING_CERT)
        set_xcode_property(${component_name} CODE_SIGN_IDENTITY "${SIGNING_CERT}")
    endif()
    # Other signing attributes of interest
    # CODE_SIGN_ENTITLEMENTS my.entitlements
    # PRODUCT_NAME "My Product"
    # DEVELOPMENT_TEAM ${DEVELOPMENT_TEAM_ID}
    # PROVISIONING_PROFILE_SPECIFIER my_profile_name
endif()

target_enable_warnings(${component_name})

install(TARGETS ${component_name} 
        DESTINATION ${CM_THIRDPARTY_EXPORT}/bin/
        COMPONENT installstaging)

# Include tests
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

