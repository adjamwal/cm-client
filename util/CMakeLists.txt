# Copyright (c) 2023 Cisco Systems, Inc. All Rights Reserved.

set(component_name "util")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(${component_name} STATIC
    util.hpp
    GuidUtil.hpp
    PathUtil.hpp
    $<$<BOOL:${APPLE}>:macOS/GuidUtil.mm>
    $<$<BOOL:${APPLE}>:macOS/StringUtil.mm>
    $<$<BOOL:${APPLE}>:macOS/PathUtil.mm>
    $<$<BOOL:${LINUX}>:linux/GuidUtil.cpp>
    $<$<BOOL:${LINUX}>:linux/PathUtil.cpp>
)

target_compile_definitions(${component_name}
    PUBLIC
    CM_BUILD_VER=\"$ENV{CM_BUILD_VER}\"
    PRIVATE
        UTIL_MODULE_API_EXPORTS
)

if(APPLE)
    set_property(TARGET ${component_name} APPEND_STRING PROPERTY 
                COMPILE_FLAGS "-fobjc-arc")

    target_link_libraries(${component_name}
        PRIVATE
            "-framework Foundation"
    )
endif()

target_include_directories(${component_name}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS ${component_name} DESTINATION ${CM_THIRDPARTY_EXPORT}/lib)
