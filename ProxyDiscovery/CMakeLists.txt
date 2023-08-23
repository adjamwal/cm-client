# Copyright (c) 2023 Cisco Systems, Inc. All Rights Reserved.

set(CMAKE_INCLUDE_CURRENT_DIR ON)

list(APPEND PROXY_DISCOVERY_SOURCES
    include/IProxyDiscoveryEngine.h
    include/IProxyLogger.h
    include/ProxyDef.h
    include/ProxyRecord.h
    src/ProxyDiscoveryEngine.h
    src/ProxyDiscoveryEngine.mm
    src/ScopedGuard.hpp
    src/ProxyLogger.cpp
    src/ProxyLoggerDef.hpp
)

add_library(ProxyDiscovery STATIC
    ${PROXY_DISCOVERY_SOURCES}
)

target_compile_definitions(ProxyDiscovery
    PRIVATE
        PROXY_DISCOVERY_MODULE_API_EXPORTS
)

set_property(TARGET ProxyDiscovery APPEND_STRING PROPERTY 
              COMPILE_FLAGS "-fobjc-arc")

target_include_directories(ProxyDiscovery
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${cm-client_SOURCE_DIR}
)

target_link_libraries(ProxyDiscovery
    PRIVATE
        util
        "-framework Foundation"
        "-framework SystemConfiguration"
)

install(TARGETS ProxyDiscovery DESTINATION ${CM_THIRDPARTY_EXPORT}/lib)
