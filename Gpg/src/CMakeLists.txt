# Copyright Cisco Systems, Inc. 2025

set(component_name "gpg")

add_library(${component_name} STATIC
    GpgKeychain.cpp
    GpgKeyId.cpp
    TemporaryDirectory.cpp
)

target_include_directories(${component_name} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

if(GPGME_LIB)
    target_sources(${component_name} PRIVATE
        GpgContext.cpp
        GpgError.cpp
        GpgUtil.cpp
    )
    target_link_libraries(${component_name} PRIVATE gpgme)
endif()
