include(TargetEnableWarnings)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CM_CLIENT_DAEMON_SRCS
    Daemon.cpp
    main.cpp
)

add_executable(cm-client-daemon ${CM_CLIENT_DAEMON_SRCS})

target_include_directories(cm-client-daemon
    PUBLIC
    ${cm-client_SOURCE_DIR}
)

#target_link_libraries(ampkit-client
#    PUBLIC
#    ${EXTRA_LIBRARIES}
#)

install(TARGETS cm-client-daemon DESTINATION ${cm-client_SOURCE_DIR}/export/bin/)
