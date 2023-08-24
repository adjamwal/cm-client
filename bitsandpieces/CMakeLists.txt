# Copyright (c) 2023 Cisco Systems, Inc. All Rights Reserved.

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(lib bitsandpieces)

list(APPEND ${lib}_SOURCES
    ConfigWatchdog.cpp
)

add_library(${lib} STATIC
    ${${lib}_SOURCES}
)

target_compile_definitions(${lib}
    PRIVATE
        ${lib}_MODULE_API_EXPORTS
)

set_target_properties(${lib} PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(${lib}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS ${lib} DESTINATION ${CM_THIRDPARTY_EXPORT}/lib)
