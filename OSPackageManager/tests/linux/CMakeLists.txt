set(component_name "package-util-test")

if (${is_rhel_based}) 
    add_executable(${component_name}
        TestPackageUtilRPM.cpp
        ../../linux/PackageUtilRPM.cpp
        ../../linux/PmPlatformConfiguration.cpp
        ../../common/PmLogger.cpp
        ../../../util/linux/GuidUtil.cpp
    )

    add_dependencies(${component_name}
        third-party-PackageManager
        third-party-gtest
    )

    target_link_directories(${component_name} BEFORE
        PRIVATE
        ${PROJECT_SOURCE_DIR}/debug/export/lib
    )

    target_link_libraries(${component_name}
        gpg
        pthread
        stdc++fs
        ${GTEST_LIBS}
        ${CMAKE_DL_LIBS}
        ProxyDiscovery
        pmutil
        util
        configshared
        curl
        ssl
        crypto
        z
    )

    target_include_directories(${component_name} PUBLIC
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/debug/export/include
        ${PROJECT_SOURCE_DIR}/OSPackageManager/linux
        ${PROJECT_SOURCE_DIR}/OSPackageManager/common
        ${PROJECT_SOURCE_DIR}/OSPackageManager/proxy
        ${PROJECT_SOURCE_DIR}/util
        ${PROJECT_SOURCE_DIR}/ConfigShared
        ${PROJECT_SOURCE_DIR}/ProxyDiscovery-Mac/src/linux
        ${PROJECT_SOURCE_DIR}/ProxyDiscovery-Mac/include
    )
else()
    # TODO: debian package util tests
endif()