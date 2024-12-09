# boost external project build file
# Copyright Cisco Systems, Inc. 2024

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
        CONFIGURE_COMMAND ./bootstrap.sh --with-libraries=chrono,date_time,filesystem,regex,system,thread
        BUILD_COMMAND ./b2 --build-dir=${component_dst_dir} --stagedir=${component_dst_dir} link=static cxxflags=-fPIC --without-python
        INSTALL_COMMAND ./b2 link=static install --prefix=${component_install_prefix} --build-dir=${component_dst_dir}
        BUILD_IN_SOURCE ON
    )

    upload_component(${component_name} not_used)
endif()

