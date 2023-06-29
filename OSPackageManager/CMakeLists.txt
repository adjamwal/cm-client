# CMakeLists.txt
# Copyright 2022, Cisco Systems, Inc.

include(TargetEnableWarnings)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(OS_PACKAGE_MANAGER_SRCS
    Daemon.cpp
    Daemon.hpp
    main.cpp
    agent/PackageManagerAgent.cpp
    agent/PackageManagerAgent.hpp
    macOS/IPmCertRetriever.hpp
    macOS/PmCertManager.cpp
    macOS/PmCertManager.hpp
    macOS/PmCertRetrieverImpl.cpp
    macOS/PmCertRetrieverImpl.hpp
    macOS/PmLogger.cpp
    macOS/PmLogger.hpp
    macOS/PmPlatformComponentManager.cpp
    macOS/PmPlatformComponentManager.hpp
    macOS/PmPlatformConfiguration.cpp
    macOS/PmPlatformConfiguration.hpp
    macOS/PmPlatformDependencies.cpp
    macOS/PmPlatformDependencies.hpp
)

add_executable(cmpackagemanager ${OS_PACKAGE_MANAGER_SRCS})

add_dependencies(cmpackagemanager
    third-party-ciscossl
    third-party-curl
    third-party-gtest
    third-party-jsoncpp
    third-party-EndpointIdentity
    third-party-PackageManager
    third-party-spdlog
)

target_include_directories(cmpackagemanager
    PUBLIC
    ${CM_THIRDPARTY_EXPORT}/include
    # This include should be based on platform when we extend support for Linux
    ${CMAKE_CURRENT_SOURCE_DIR}/macOS
)

target_link_directories(cmpackagemanager BEFORE
    PRIVATE
    ${CM_THIRDPARTY_EXPORT}/lib
)

target_link_libraries(cmpackagemanager
    PUBLIC
    cmidapi
    pmclient
    pmcloud
    pmconfig
    pmutil
    jsoncpp
    curl
    crypto
    ssl
    z
    "-framework SystemConfiguration"
    "-framework IOKit"
    "-framework CoreServices"
    "-framework Security"
)

target_compile_definitions(cmpackagemanager
    PUBLIC
    "$<$<CONFIG:DEBUG>:DEBUG>"
    "$<$<CONFIG:DEBUG>:CMID_DAEMON_PATH=\"${CM_THIRDPARTY_EXPORT}/bin\">"
    "$<$<CONFIG:DEBUG>:CM_CONFIG_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/client/config\">"
    "$<$<CONFIG:DEBUG>:CM_SHARED_LOG_PATH=\"${CMAKE_CURRENT_BINARY_DIR}/\">"
    "$<$<CONFIG:DEBUG>:PackageManager_VERSION_MAJOR=\"1\">"
    "$<$<CONFIG:DEBUG>:PackageManager_VERSION_MINOR=\"0\">"
)

if(CMAKE_GENERATOR STREQUAL "Xcode")
    macro(set_xcode_property TARGET XCODE_PROPERTY XCODE_VALUE)
    set_property (TARGET ${TARGET} PROPERTY XCODE_ATTRIBUTE_${XCODE_PROPERTY}
        ${XCODE_VALUE})
    endmacro (set_xcode_property)

    set_xcode_property(cmpackagemanager CLANG_CXX_LANGUAGE_STANDARD "gnu++17")
    set_xcode_property(cmpackagemanager CLANG_CXX_LIBRARY "libc++")
    set_xcode_property(cmpackagemanager DEPLOYMENT_LOCATION "YES")
    set_xcode_property(cmpackagemanager INSTALL_PATH "/")
    set_xcode_property(cmpackagemanager DSTROOT "${CM_THIRDPARTY_EXPORT}/bin")
    
    # For code signing
    if(DEFINED SIGNING_CERT)
        set_xcode_property(cmpackagemanager CODE_SIGN_IDENTITY "${SIGNING_CERT}")
    endif()
    # Other signing attributes of interest
    # CODE_SIGN_ENTITLEMENTS my.entitlements
    # PRODUCT_NAME "My Product"
    # DEVELOPMENT_TEAM ${DEVELOPMENT_TEAM_ID}
    # PROVISIONING_PROFILE_SPECIFIER my_profile_name
endif()

target_enable_warnings(cmpackagemanager)

install(TARGETS cmpackagemanager DESTINATION ${CM_THIRDPARTY_EXPORT}/bin/)

add_subdirectory(controlplugin)

# Include tests
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
