set(component_name "package-util-test")

if (${is_rhel_based}) 
    add_executable(${component_name}
        TestPackageUtilRPM.cpp
        ../../linux/PackageUtilRPM.cpp
        ../../common/PmLogger.cpp
    )

    add_dependencies(${component_name}
        third-party-PackageManager
        third-party-gtest
    )

    target_link_libraries(${component_name}
        gpg
        pthread
        ${GTEST_LIBS}
        ${CMAKE_DL_LIBS}
    )

    target_include_directories(${component_name} PUBLIC
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/debug/export/include
        ${PROJECT_SOURCE_DIR}/OSPackageManager/linux
        ${PROJECT_SOURCE_DIR}/OSPackageManager/common
        ${PROJECT_SOURCE_DIR}/ConfigShared
    )
else()
    # TODO: debian package util tests
endif()
