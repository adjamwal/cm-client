# Determine the current architecture
# Copyright Cisco Systems, Inc. 2024

set(arch ${CMAKE_HOST_SYSTEM_PROCESSOR})
set(ENV{ARCH} ${arch})

if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64")
    set(is_arch_intel true)
else()
    set(is_arch_intel false)
endif()

if((CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "aarch64") OR (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "arm64"))
    set(is_arch_arm true)
else()
    set(is_arch_arm false)
endif()

if(is_arch_arm AND is_arch_intel)
    message(FATAL_ERROR 
        "Evaluated architecture (${arch}) as intel (${is_arch_intel}) and ARM (${is_arch_arm}). This shouldn't happen!\n"
        "Evaluation takes place in: cmake/Modules/helper/determine_architecture.cmake")
endif()

if((NOT is_arch_arm) AND (NOT is_arch_intel))
    message(FATAL_ERROR 
        "Evaluated architecture (${arch}) as intel (${is_arch_intel}) and ARM (${is_arch_arm}). This shouldn't happen!\n"
        "Evaluation takes place in: ampcx/cmake/modules/helper/determine_architecture.cmake")
endif()

message(STATUS "Architecture: ${arch} detected.")
