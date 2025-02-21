# Copyright (c) 2023 Cisco Systems, Inc. All Rights Reserved.

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(component_name "configshared")

add_library(${component_name} STATIC
    ConfigWatchdog.cpp
    Config.cpp
    Config.hpp
    IConfigLogger.hpp
)

add_dependencies(${component_name}
    third-party-jsoncpp
 )


target_link_libraries(${component_name}
    PUBLIC
    jsoncpp
)

target_compile_definitions(${component_name}
    PRIVATE
    ${component_name}_MODULE_API_EXPORTS
    "$<$<CONFIG:DEBUG>:DEBUG>"
    "$<$<CONFIG:DEBUG>:CMID_DAEMON_PATH=\"${CM_THIRDPARTY_EXPORT}/bin\">"
    "$<$<CONFIG:DEBUG>:CM_CONFIG_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/config\">"
    "$<$<CONFIG:DEBUG>:CM_SHARED_LOG_PATH=\"${CMAKE_CURRENT_BINARY_DIR}/\">"
    "$<$<CONFIG:DEBUG>:CMID_LOG_PATH=\"/Library/Logs/Cisco/SecureClient/CloudManagement/\">"

)


set_target_properties(${component_name} PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(${component_name}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
    ${CM_THIRDPARTY_EXPORT}/include

)

install(TARGETS ${component_name} DESTINATION ${CM_THIRDPARTY_EXPORT}/lib)
