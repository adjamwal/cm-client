# CMakeLists.txt
# Copyright 2022, Cisco Systems, Inc.

include(TargetEnableWarnings)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(PM_KEY "pm")

if(APPLE)
set(OS_PACKAGE_MANAGER_SRCS
    Daemon.cpp
    Daemon.hpp
    main.cpp
    agent/PackageManagerAgent.cpp
    agent/PackageManagerAgent.hpp
    common/PmLogger.cpp
    common/PmLogger.hpp
    macOS/IFileUtilities.hpp
    macOS/IPmCertRetriever.hpp
    macOS/FileUtilities.cpp
    macOS/FileUtilities.mm
    macOS/FileUtilities.hpp
    macOS/PmCertManager.cpp
    macOS/PmCertManager.hpp
    macOS/PmCertRetrieverImpl.cpp
    macOS/PmCertRetrieverImpl.hpp
    macOS/PmPlatformComponentManager.cpp
    macOS/PmPlatformComponentManager.hpp
    macOS/PmPlatformConfiguration.cpp
    macOS/PmPlatformConfiguration.hpp
    macOS/PmPlatformDependencies.cpp
    macOS/PmPlatformDependencies.hpp
    macOS/PmPkgUtilWrapper.cpp
    macOS/PmPkgUtilWrapper.hpp
    macOS/PmPlatformDiscovery.hpp
    macOS/PmPlatformDiscovery.cpp
    macOS/PmCodesignVerifierMac.cpp
    macOS/util/codesign_pkg.mm
    macOS/util/codesign_bin.cpp
    ${cm-client_SOURCE_DIR}/common/macOS/FileWatcher.mm
    ${cm-client_SOURCE_DIR}/common/macOS/FileWatcher.hpp
)
else()
set(OS_PACKAGE_MANAGER_SRCS
    Daemon.cpp
    Daemon.hpp
    main.cpp
    common/PmLogger.cpp
    common/PmLogger.hpp
    agent/PackageManagerAgent.cpp
    agent/PackageManagerAgent.hpp
    common/CommandExec.cpp
    common/CommandExec.hpp
    $<$<BOOL:${is_rhel_based}>:linux/PackageUtilRPM.cpp>
    $<$<BOOL:${is_rhel_based}>:linux/PackageUtilRPM.hpp>
    linux/PmPlatformComponentManager.cpp
    linux/PmPlatformComponentManager.hpp
    linux/PmPlatformConfiguration.cpp
    linux/PmPlatformConfiguration.hpp
    linux/PmPlatformDependencies.cpp
    linux/PmPlatformDependencies.hpp
)
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/common/PmLogger.cpp PROPERTIES COMPILE_FLAGS "-Wno-maybe-uninitialized")
endif()

add_executable(cmpackagemanager ${OS_PACKAGE_MANAGER_SRCS})

if(APPLE)
set_property(TARGET cmpackagemanager APPEND_STRING PROPERTY 
              COMPILE_FLAGS "-fobjc-arc")
endif()

add_dependencies(cmpackagemanager
    third-party-ciscossl
    third-party-curl
    third-party-gtest
    third-party-jsoncpp
    third-party-EndpointIdentity
    third-party-PackageManager
    third-party-spdlog
)

if(APPLE)
add_dependencies(cmpackagemanager
    third-party-ProxyDiscovery-Mac
)
endif()

target_include_directories(cmpackagemanager
    PUBLIC
    ${CM_THIRDPARTY_EXPORT}/include
    $<$<BOOL:${APPLE}>:${CM_THIRDPARTY_EXPORT}/include/ProxyDiscovery>
    ${cm-client_SOURCE_DIR}
    ${cm-client_SOURCE_DIR}/util
    # This include should be based on platform when we extend support for Linux
    $<$<BOOL:${APPLE}>:${CMAKE_CURRENT_SOURCE_DIR}/macOS>
    $<$<BOOL:${LINUX}>:${CMAKE_CURRENT_SOURCE_DIR}/linux>
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/util
    ${CMAKE_CURRENT_SOURCE_DIR}/proxy
)

target_link_directories(cmpackagemanager BEFORE
    PRIVATE
    ${CM_THIRDPARTY_EXPORT}/lib
)

target_link_libraries(cmpackagemanager
    PUBLIC
    $<$<BOOL:${APPLE}>:ProxyDiscovery>
    cmidapi
    pmclient
    pmcloud
    pmconfig
    pmutil
    jsoncpp
    curl
    crypto
    ssl
    util
    configshared
    pthread
    z
    $<$<BOOL:${LINUX}>:stdc++fs>
    $<$<BOOL:${LINUX}>:dl>
    $<$<BOOL:${APPLE}>:xar>
    "$<$<BOOL:${APPLE}>:-framework SystemConfiguration>"
    "$<$<BOOL:${APPLE}>:-framework IOKit>"
    "$<$<BOOL:${APPLE}>:-framework CoreServices>"
    "$<$<BOOL:${APPLE}>:-framework Security>"
)

if(LINUX)
set_target_properties(cmpackagemanager
PROPERTIES
    INSTALL_RPATH "$ORIGIN/../lib"
    INSTALL_RPATH_USE_LINK_PATH TRUE
)
endif()

target_compile_definitions(cmpackagemanager
    PUBLIC
    "$<$<CONFIG:DEBUG>:DEBUG>"
    "$<$<CONFIG:DEBUG>:CMID_DAEMON_PATH=\"${CM_THIRDPARTY_EXPORT}/bin\">"
    "$<$<CONFIG:DEBUG>:CM_CONFIG_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/client/config\">"
    "$<$<CONFIG:DEBUG>:CM_SHARED_LOG_PATH=\"${CMAKE_CURRENT_BINARY_DIR}/\">"
    PackageManager_VERSION_MAJOR=\"1\"
    PackageManager_VERSION_MINOR=\"0\"
    $<IF:${LOCAL_WEBSERVER_OVERRIDE},LOCAL_WEBSERVER_OVERRIDE=1,LOCAL_WEBSERVER_OVERRIDE=0>
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

install(TARGETS cmpackagemanager 
        DESTINATION ${CM_THIRDPARTY_EXPORT}/bin/
        COMPONENT installstaging)

add_subdirectory(controlplugin)

# Include tests
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
