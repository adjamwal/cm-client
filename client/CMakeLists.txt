# CMakeLists.txt
# Copyright 2022, Cisco Systems, Inc.

include(TargetEnableWarnings)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CM_CLIENT_DAEMON_SRCS
    Daemon.cpp
    Daemon.hpp
    main.cpp
#    ComponentLoader/CMIDLoader.hpp
#    ComponentLoader/CMIDLoader.cpp
    Configuration/Config.hpp
    Configuration/Config.cpp
)

add_executable(cm-client-daemon ${CM_CLIENT_DAEMON_SRCS})

add_dependencies(cm-client-daemon
    third-party-ciscossl
    third-party-curl
    third-party-gtest
    third-party-jsoncpp
    third-party-cmid
    third-party-PackageManager
)

target_include_directories(cm-client-daemon
    PUBLIC
    ${cm-client_SOURCE_DIR}
    ${CM_THIRDPARTY_EXPORT}/include
)

target_link_directories(cm-client-daemon BEFORE
    PRIVATE
    ${CM_THIRDPARTY_EXPORT}/lib
)

target_link_libraries(cm-client-daemon
    PUBLIC
    cmidcontrolplugin
    common
    "-framework SystemConfiguration"
    "-framework IOKit"
    "-framework CoreServices"
    "-framework Security"
)

target_compile_definitions(cm-client-daemon
    PUBLIC
    "$<$<CONFIG:DEBUG>:DEBUG>"
    "$<$<CONFIG:DEBUG>:CMID_DAEMON_PATH=\"${CM_THIRDPARTY_EXPORT}/bin\">"
    "$<$<CONFIG:DEBUG>:CM_CONFIG_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/client/config\">"
    "$<$<CONFIG:DEBUG>:CM_SHARED_LOG_PATH=\"${CMAKE_CURRENT_BINARY_DIR}/\">"
)

target_enable_warnings(cm-client-daemon)

install(TARGETS cm-client-daemon DESTINATION ${cm-client_SOURCE_DIR}/export/bin/)
