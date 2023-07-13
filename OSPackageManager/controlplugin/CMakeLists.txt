# Copyright (c) 2023 Cisco Systems, Inc. All Rights Reserved.

list(APPEND CONTROL_PLUGIN_SOURCES
    ModuleControlPlugin.cpp
    ModuleControlPlugin.hpp
    PmAgentController.cpp
    PmAgentController.hpp
    PmStatusTypes.hpp
    IProcessWrapper.hpp
    ProcessWrapper.cpp
    ProcessWrapper.hpp
    ExecutionError.hpp
    ExecutionError.cpp
    ../util/ScopedGuard.hpp
)

add_library(pmcontrolplugin STATIC
    ${CONTROL_PLUGIN_SOURCES}
)

target_compile_definitions(pmcontrolplugin
    PRIVATE
        PM_MODULE_API_EXPORTS
)

target_include_directories(pmcontrolplugin
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CM_THIRDPARTY_EXPORT}/include
        ${cm-client_SOURCE_DIR}/client/Logger
)

add_dependencies(pmcontrolplugin 
        third-party-EndpointIdentity
        third-party-spdlog
)

target_link_libraries(pmcontrolplugin
    PRIVATE
        ${SystemConfiguration_framework}
        ${IOKit_framework}
        ${CoreServices_framework}
        ${Security_framework}
)

install(TARGETS pmcontrolplugin DESTINATION ${CM_THIRDPARTY_EXPORT}/lib/)
