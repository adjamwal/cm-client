# Copyright (c) 2023 Cisco Systems, Inc. All Rights Reserved.

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(lib configshared)

list(APPEND ${lib}_SOURCES
    ConfigWatchdog.cpp
    Config.cpp
    Config.hpp
    IConfigLogger.hpp
)

add_library(${lib} STATIC
    ${${lib}_SOURCES}
)

add_dependencies(${lib}
    third-party-jsoncpp
 )


target_link_libraries(${lib}
    PUBLIC
    jsoncpp
)

target_compile_definitions(${lib}
    PRIVATE
    ${lib}_MODULE_API_EXPORTS
    "$<$<CONFIG:DEBUG>:DEBUG>"
    "$<$<CONFIG:DEBUG>:CMID_DAEMON_PATH=\"${CM_THIRDPARTY_EXPORT}/bin\">"
    "$<$<CONFIG:DEBUG>:CM_CONFIG_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/config\">"
    "$<$<CONFIG:DEBUG>:CM_SHARED_LOG_PATH=\"${CMAKE_CURRENT_BINARY_DIR}/\">"
    "$<$<CONFIG:DEBUG>:CMID_LOG_PATH=\"/Library/Logs/Cisco/SecureClient/CloudManagement/\">"

)


set_target_properties(${lib} PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(${lib}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CM_THIRDPARTY_EXPORT}/include

)

install(TARGETS ${lib} DESTINATION ${CM_THIRDPARTY_EXPORT}/lib)
