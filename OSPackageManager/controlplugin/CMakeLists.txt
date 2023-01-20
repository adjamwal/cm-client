# Copyright (c) 2023 Cisco Systems, Inc. All Rights Reserved.

list(APPEND CONTROL_PLUGIN_SOURCES
    ModuleControlPlugin.cpp
    PmAgentController.cpp
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
        ${CM_THIRDPARTY_EXPORT}/include
)

target_link_libraries(pmcontrolplugin
    PRIVATE
        ${SystemConfiguration_framework}
        ${IOKit_framework}
        ${CoreServices_framework}
        ${Security_framework}
)

install(TARGETS pmcontrolplugin DESTINATION ${CM_THIRDPARTY_EXPORT}/lib/)