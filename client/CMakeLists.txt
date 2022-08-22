# CMakeLists.txt
# Copyright 2022, Cisco Systems, Inc.

include(TargetEnableWarnings)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CM_CLIENT_DAEMON_SRCS
    Daemon.cpp
    main.cpp
)

add_executable(cm-client-daemon ${CM_CLIENT_DAEMON_SRCS})

add_dependencies(cm-client-daemon third-party-jsoncpp)

target_include_directories(cm-client-daemon
    PUBLIC
    ${cm-client_SOURCE_DIR}
)

#target_link_libraries(cm-client-daemon
#    PUBLIC
#    ${EXTRA_LIBRARIES}
#)

install(TARGETS cm-client-daemon DESTINATION ${cm-client_SOURCE_DIR}/export/bin/)
