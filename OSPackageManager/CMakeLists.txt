# CMakeLists.txt
# Copyright 2022, Cisco Systems, Inc.

include(TargetEnableWarnings)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(PM_KEY "pm")

set(component_name "cmpackagemanager")

add_executable(${component_name} 
    Daemon.cpp
    Daemon.hpp
    main.cpp
    agent/PackageManagerAgent.cpp
    agent/PackageManagerAgent.hpp
    common/PmLogger.cpp
    common/PmLogger.hpp
    common/IFileUtilities.hpp
)

if(APPLE)
    target_sources(${component_name} PRIVATE
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
    set_property(TARGET ${component_name} APPEND_STRING PROPERTY COMPILE_FLAGS "-fobjc-arc")
    add_dependencies(${component_name}
        third-party-ProxyDiscovery-Mac
    )
else()
    target_sources(${component_name} PRIVATE
        common/CommandExec.cpp
        common/CommandExec.hpp
        linux/FileUtilities.cpp
        linux/FileUtilities.hpp
        linux/IPmCertRetriever.hpp
        linux/PmCertManager.cpp
        linux/PmCertManager.hpp
        linux/PmCertRetrieverImpl.cpp
        linux/PmCertRetrieverImpl.hpp
        $<$<BOOL:${is_rhel_based}>:linux/PackageUtilRPM.cpp>
        $<$<BOOL:${is_rhel_based}>:linux/PackageUtilRPM.hpp>
        $<$<BOOL:${is_debian_based}>:linux/PackageUtilDEB.cpp>
        $<$<BOOL:${is_debian_based}>:linux/PackageUtilDEB.hpp>
        linux/PmPlatformComponentManager.cpp
        linux/PmPlatformComponentManager.hpp
        linux/PmPlatformConfiguration.cpp
        linux/PmPlatformConfiguration.hpp
        linux/PmPlatformDependencies.cpp
        linux/PmPlatformDependencies.hpp
        linux/PmPlatformDiscovery.hpp
        linux/PmPlatformDiscovery.cpp
    )
    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/common/PmLogger.cpp PROPERTIES COMPILE_FLAGS "-Wno-maybe-uninitialized")
endif()

add_dependencies(${component_name}
    third-party-ciscossl
    third-party-curl
    third-party-gtest
    third-party-jsoncpp
    third-party-EndpointIdentity
    third-party-PackageManager
    third-party-spdlog
)

target_include_directories(${component_name}
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

target_link_directories(${component_name} BEFORE
    PRIVATE
    ${CM_THIRDPARTY_EXPORT}/lib
)

target_link_libraries(${component_name}
    PUBLIC
    $<$<BOOL:${APPLE}>:ProxyDiscovery>
    cmidapi
    pmclient
    pmcloud
    pmconfig
    pmutil
    jsoncpp
    curl
    ssl # Order should be maintained for ssl and crypto
    crypto
    util
    configshared
    pthread
    z
    $<$<BOOL:${LINUX}>:gpg>
    $<$<BOOL:${LINUX}>:stdc++fs>
    $<$<BOOL:${LINUX}>:dl>
    $<$<BOOL:${APPLE}>:xar>
    "$<$<BOOL:${APPLE}>:-framework SystemConfiguration>"
    "$<$<BOOL:${APPLE}>:-framework IOKit>"
    "$<$<BOOL:${APPLE}>:-framework CoreServices>"
    "$<$<BOOL:${APPLE}>:-framework Security>"
)

if(LINUX)
    set_target_properties(${component_name}
        PROPERTIES
            INSTALL_RPATH "$ORIGIN/../lib"
            INSTALL_RPATH_USE_LINK_PATH TRUE
    )
endif()

target_compile_definitions(${component_name}
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

    set_xcode_property(${component_name} CLANG_CXX_LANGUAGE_STANDARD "gnu++17")
    set_xcode_property(${component_name} CLANG_CXX_LIBRARY "libc++")
    set_xcode_property(${component_name} DEPLOYMENT_LOCATION "YES")
    set_xcode_property(${component_name} INSTALL_PATH "/")
    set_xcode_property(${component_name} DSTROOT "${CM_THIRDPARTY_EXPORT}/bin")
    
    # For code signing
    if(DEFINED SIGNING_CERT)
        set_xcode_property(${component_name} CODE_SIGN_IDENTITY "${SIGNING_CERT}")
    endif()
    # Other signing attributes of interest
    # CODE_SIGN_ENTITLEMENTS my.entitlements
    # PRODUCT_NAME "My Product"
    # DEVELOPMENT_TEAM ${DEVELOPMENT_TEAM_ID}
    # PROVISIONING_PROFILE_SPECIFIER my_profile_name
endif()

target_enable_warnings(${component_name})

install(TARGETS ${component_name} 
        DESTINATION ${CM_THIRDPARTY_EXPORT}/bin/
        COMPONENT installstaging)

if(LINUX AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    add_custom_command(TARGET ${component_name} POST_BUILD 
        COMMAND cp "${PROJECT_SOURCE_DIR}/debug/OSPackageManager/cmpackagemanager" "${CM_THIRDPARTY_EXPORT}/bin/cmpackagemanager"
    )
endif()

add_subdirectory(controlplugin)

# Include tests
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
