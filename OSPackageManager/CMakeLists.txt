# CMakeLists.txt
# Copyright 2022, Cisco Systems, Inc.

include(TargetEnableWarnings)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(PM_CLIENT_PROCESS_SRCS
    Daemon.cpp
    Daemon.hpp
    main.cpp
    macOS/PmLogger.cpp
    macOS/PmLogger.hpp
    macOS/PmPlatformComponentManager.cpp
    macOS/PmPlatformComponentManager.hpp
    macOS/PmPlatformConfiguration.cpp
    macOS/PmPlatformConfiguration.hpp
    macOS/PmPlatformDependencies.cpp
    macOS/PmPlatformDependencies.hpp
)

add_executable(cm-pm-client ${PM_CLIENT_PROCESS_SRCS})

add_dependencies(cm-pm-client
    third-party-ciscossl
    third-party-curl
    third-party-gtest
    third-party-jsoncpp
    third-party-cmid
    third-party-PackageManager
)

target_include_directories(cm-pm-client
    PUBLIC
    ${CM_THIRDPARTY_EXPORT}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/cm-pm-client/macOS
)

target_link_directories(cm-pm-client BEFORE
    PRIVATE
    ${CM_THIRDPARTY_EXPORT}/lib
)

target_link_libraries(cm-pm-client
    PUBLIC
    cmidapi
    common
    "-framework SystemConfiguration"
    "-framework IOKit"
    "-framework CoreServices"
    "-framework Security"
)

target_compile_definitions(cm-pm-client
    PUBLIC
    "$<$<CONFIG:DEBUG>:DEBUG>"
    "$<$<CONFIG:DEBUG>:CMID_DAEMON_PATH=\"${CM_THIRDPARTY_EXPORT}/bin\">"
    "$<$<CONFIG:DEBUG>:CM_CONFIG_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/client/config\">"
    "$<$<CONFIG:DEBUG>:CM_SHARED_LOG_PATH=\"${CMAKE_CURRENT_BINARY_DIR}/\">"
)

if(CMAKE_GENERATOR STREQUAL "Xcode")
    macro(set_xcode_property TARGET XCODE_PROPERTY XCODE_VALUE)
    set_property (TARGET ${TARGET} PROPERTY XCODE_ATTRIBUTE_${XCODE_PROPERTY}
        ${XCODE_VALUE})
    endmacro (set_xcode_property)

    set_xcode_property(cm-pm-client CLANG_CXX_LANGUAGE_STANDARD "gnu++17")
    set_xcode_property(cm-pm-client CLANG_CXX_LIBRARY "libc++")
endif()

target_enable_warnings(cm-pm-client)

install(TARGETS cm-pm-client DESTINATION ${CM_THIRDPARTY_EXPORT}/export/bin/)
