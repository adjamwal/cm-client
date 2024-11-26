# jsoncpp external project build file
# Copyright Cisco Systems, Inc. 2022

include(ExternalProject)

set(component_name boost)
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
        INSTALL_DIR ${component_install_prefix}
        CONFIGURE_COMMAND ./bootstrap.sh
        BUILD_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/third-party/${component_name}/b2 link=static cxxflags=-fPIC --with-chrono --with-date_time --with-filesystem --with-system --with-thread --with-regex
        INSTALL_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/third-party/${component_name}/b2 link=static install --prefix=${component_install_prefix}
        BUILD_IN_SOURCE 1
    )

    upload_component(${component_name} not_used)
endif()

