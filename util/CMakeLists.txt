# Copyright (c) 2023 Cisco Systems, Inc. All Rights Reserved.

set(CMAKE_INCLUDE_CURRENT_DIR ON)

list(APPEND UTIL_SOURCES
    util.hpp
    GuidUtil.hpp
    PathUtil.hpp
    $<$<BOOL:${APPLE}>:macOS/GuidUtil.mm>
    $<$<BOOL:${APPLE}>:macOS/StringUtil.mm>
    $<$<BOOL:${APPLE}>:macOS/PathUtil.mm>
    $<$<BOOL:${LINUX}>:linux/GuidUtil.cpp>
    $<$<BOOL:${LINUX}>:linux/PathUtil.cpp>
)

add_library(util STATIC
    ${UTIL_SOURCES}
)

target_compile_definitions(util
    PUBLIC
    CM_BUILD_VER=\"$ENV{CM_BUILD_VER}\"
    PRIVATE
        UTIL_MODULE_API_EXPORTS
)

if(APPLE)
set_property(TARGET util APPEND_STRING PROPERTY 
              COMPILE_FLAGS "-fobjc-arc")

target_link_libraries(util
    PRIVATE
        "-framework Foundation"
)
endif()

target_include_directories(util
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS util DESTINATION ${CM_THIRDPARTY_EXPORT}/lib)
