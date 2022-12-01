# CMakeLists.txt
# Copyright 2022, Cisco Systems, Inc.

include(TargetEnableWarnings)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CM_CLIENT_SRCS
    Daemon.cpp
    Daemon.hpp
    main.cpp
    ComponentLoader/CMIDLoader.hpp
    ComponentLoader/CMIDLoader.cpp
    Configuration/Config.hpp
    Configuration/Config.cpp
)

add_executable(csccloudmanagement ${CM_CLIENT_SRCS})

add_dependencies(csccloudmanagement
    third-party-ciscossl
    third-party-curl
    third-party-gtest
    third-party-jsoncpp
    third-party-cmid
    third-party-PackageManager
)

target_include_directories(csccloudmanagement
    PUBLIC
    ${cm-client_SOURCE_DIR}
    ${CM_THIRDPARTY_EXPORT}/include
)

target_link_directories(csccloudmanagement BEFORE
    PRIVATE
    ${CM_THIRDPARTY_EXPORT}/lib
)

target_link_libraries(csccloudmanagement
    PUBLIC
    cmidcontrolplugin
    common
    jsoncpp
    "-framework SystemConfiguration"
    "-framework IOKit"
    "-framework CoreServices"
    "-framework Security"
)

target_compile_definitions(csccloudmanagement
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

    set_xcode_property(csccloudmanagement CLANG_CXX_LANGUAGE_STANDARD "gnu++17")
    set_xcode_property(csccloudmanagement CLANG_CXX_LIBRARY "libc++")
endif()

target_enable_warnings(csccloudmanagement)

install(TARGETS csccloudmanagement DESTINATION ${CM_THIRDPARTY_EXPORT}/bin/)
