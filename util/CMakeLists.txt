# Copyright (c) 2023 Cisco Systems, Inc. All Rights Reserved.

set(CMAKE_INCLUDE_CURRENT_DIR ON)

list(APPEND UTIL_SOURCES
    util.hpp
    GuidUtil.hpp
    macOS/GuidUtil.mm
    macOS/StringUtil.mm
)

add_library(util STATIC
    ${UTIL_SOURCES}
)

target_compile_definitions(util
    PRIVATE
        UTIL_MODULE_API_EXPORTS
)

set_property(TARGET util APPEND_STRING PROPERTY 
              COMPILE_FLAGS "-fobjc-arc")

target_include_directories(util
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(util
    PRIVATE
        "-framework Foundation"
)

install(TARGETS util DESTINATION ${CM_THIRDPARTY_EXPORT}/lib)
