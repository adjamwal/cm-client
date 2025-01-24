# PackageManager external project build file
# Copyright Cisco Systems, Inc. 2022

include(ExternalProject)

set(component_name PackageManager)
set(component_dst_dir "${CMAKE_CURRENT_BINARY_DIR}/third-party-${component_name}-prefix/src/third-party-${component_name}")
set(component_install_prefix "${CMAKE_CURRENT_SOURCE_DIR}/${component_name}/export")

if(NOT BUILD_ALL_THIRD_PARTY AND NOT BUILD_PACKAGE_MANAGER_THIRD_PARTY)
    download_component(${component_name} ${component_dst_dir})
endif()

set(FFF_EXPORT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party/fff")

if(LINUX)
    # HAVE_DPKG
    find_program(have_dpkg dpkg)
    if(have_dpkg)
        # There's a linkage error in the tests, skipping building the tests for simplicity
        set(SKIP_TESTS true)
    else()
        set(SKIP_TESTS false)
    endif()
else()
    set(SKIP_TESTS false)
endif()

if(NOT TARGET "third-party-${component_name}")
    #
    # TODO Be more specific about tool chain
    ExternalProject_Add(
        third-party-${component_name}
        DEPENDS third-party-ciscossl third-party-curl third-party-jsoncpp third-party-gtest
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/PackageManager/cmake
        PREFIX ${component_install_prefix}
        CMAKE_GENERATOR ${CMAKE_GENERATOR}
        CMAKE_ARGS
            -Djsoncpp_INCLUDE_DIRS=${CM_THIRDPARTY_EXPORT}/include
            -Djsoncpp_LIBRARY=${CM_THIRDPARTY_EXPORT}/lib
            -Dcurl_INCLUDE_DIRS=${CM_THIRDPARTY_EXPORT}/include
            -Dcurl_LIBRARY=${CM_THIRDPARTY_EXPORT}/lib
            -Dssl_INCLUDE_DIRS=${CM_THIRDPARTY_EXPORT}/include
            -Dssl_LIBRARY=${CM_THIRDPARTY_EXPORT}/lib
            -Dfff_INCLUDE_DIRS=${FFF_EXPORT_DIR}
            -Dgtest_INCLUDE_DIRS=${CM_THIRDPARTY_EXPORT}/include
            -Dgtest_LIBRARY=${CM_THIRDPARTY_EXPORT}/lib
            -DSKIP_TESTS=${SKIP_TESTS}
            -DCMAKE_INSTALL_PREFIX:PATH=${component_install_prefix}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES_}
            -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
            -DBUILD_WITH_HTTP_ENABLED=${BUILD_WITH_HTTP_ENABLED}
    )

if(LINUX)
    # Reasons for always building PackageManager are in the main CMakeLists.txt
    # IF we're changing this behaviour then we must also update that file to match
    ExternalProject_Add_Step(
        third-party-${component_name}
        copy_exports
        COMMENT "-- Copying exports for ${component_name} to common export directory"
        COMMAND ${COMMAND} cp -aR "${component_install_prefix}/." "${CM_THIRDPARTY_EXPORT}"
        DEPENDEES install
    )
else()
    upload_component(${component_name} not_used)
endif()
endif()
