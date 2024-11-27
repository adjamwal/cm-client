# libxml2 external project build file
# Copyright Cisco Systems, Inc. 2024

include(ExternalProject)

set(component_name libxml2)
set(component_dst_dir "${CMAKE_CURRENT_BINARY_DIR}/third-party-${component_name}-prefix/src/third-party-${component_name}")
set(component_install_prefix "${CMAKE_CURRENT_SOURCE_DIR}/third-party/${component_name}/export")

if(NOT BUILD_ALL_THIRD_PARTY)
    download_component(${component_name} ${component_dst_dir})
endif()

if(NOT TARGET "third-party-${component_name}")
    #
    # TODO Be more specific about tool chain
    ExternalProject_Add(
        third-party-${component_name}
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-party/${component_name}
        PREFIX ${component_install_prefix}
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX:PATH=${component_install_prefix}
	    -DCMAKE_INSTALL_LIBDIR=lib
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
	    -DBUILD_SHARED_LIBS=OFF
	    -DLIBXML2_WITH_LZMA=OFF
	    -DLIBXML2_WITH_PYTHON=OFF
    )

    upload_component(${component_name} not_used)
endif()

set(LIBXML2_INCLUDE_DIR ${component_install_prefix}/include/libxml2)
file(MAKE_DIRECTORY ${LIBXML2_INCLUDE_DIR})

add_library(${component_name} STATIC IMPORTED)
set_target_properties(${component_name} PROPERTIES
    IMPORTED_LOCATION ${component_install_prefix}/lib/libxml2.a
)
set(LIBXML2_LIBRARY ${component_install_prefix}/lib/libxml2.a)

target_include_directories(${component_name} INTERFACE
    ${LIBXML2_INCLUDE_DIR}
)
