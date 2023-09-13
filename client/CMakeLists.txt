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
    ${cm-client_SOURCE_DIR}/common/macOS/FileWatcher.mm
    ${cm-client_SOURCE_DIR}/common/macOS/FileWatcher.hpp
    crashpad/CrashpadTuner.h
    crashpad/CrashpadTuner.cpp
)

add_executable(csccloudmanagement ${CM_CLIENT_SRCS})

add_dependencies(csccloudmanagement
    third-party-ciscossl
    third-party-curl
    third-party-jsoncpp
    third-party-EndpointIdentity
    third-party-PackageManager
    third-party-spdlog
    third-party-crashpad
)

target_include_directories(csccloudmanagement
    PUBLIC
    ${cm-client_SOURCE_DIR}
    ${CM_THIRDPARTY_EXPORT}/include
    ${CM_THIRDPARTY_EXPORT}/include/crashpad
    ${CM_THIRDPARTY_EXPORT}/include/crashpad/third_party/mini_chromium/mini_chromium
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
    jsoncpp
    crashpad_client
    crashpad_common
    crashpad_context
    crashpad_handler_lib
    crashpad_minidump
    crashpad_net
    crashpad_snapshot
    crashpad_util
    mig_output
    crashpad_base
    bsm
    util
    "-framework SystemConfiguration"
    "-framework IOKit"
    "-framework CoreServices"
    "-framework CoreGraphics"
    "-framework CoreText"
    "-framework Security"
    "-framework Foundation"
)

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

install(TARGETS csccloudmanagement DESTINATION ${CM_THIRDPARTY_EXPORT}/bin/)

# Include tests
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

