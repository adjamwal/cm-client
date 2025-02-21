# CMakeLists.txt
# Copyright 2023, Cisco Systems, Inc.

set(component_name "PackageManagerUnitTest")

get_filename_component(TESTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}" DIRECTORY)
get_filename_component(TESTEE_DIR "${TESTS_DIR}" DIRECTORY)

file(GLOB_RECURSE TEST_SRCS Test*.cpp Test*.mm)

add_executable(${component_name}
  ${cm-client_SOURCE_DIR}/client/Logger/CMLogger.cpp

  # Test files
  ${TEST_SRCS}

  # Dependencies
  ${TESTEE_DIR}/macOS/PmPkgUtilWrapper.cpp
  ${TESTEE_DIR}/macOS/PmPlatformConfiguration.cpp
  ${TESTEE_DIR}/macOS/PmCertManager.cpp
  ${TESTEE_DIR}/macOS/PmPlatformComponentManager.cpp
  ${TESTEE_DIR}/macOS/PmPlatformDiscovery.cpp
  ${TESTEE_DIR}/common/PmLogger.cpp
  ${TESTEE_DIR}/common/PmLogger.hpp
  ${TESTEE_DIR}/macOS/PmCodesignVerifierMac.cpp
  ${TESTEE_DIR}/macOS/FileUtilities.cpp
  ${TESTEE_DIR}/macOS/FileUtilities.mm
  ${TESTEE_DIR}/macOS/util/codesign_pkg.mm
  ${TESTEE_DIR}/macOS/util/codesign_bin.cpp
  ${cm-client_SOURCE_DIR}/common/macOS/FileWatcher.mm

  # Mocks
  ${TESTEE_DIR}/proxy/CMIDAPIProxy.hpp
  ${TESTEE_DIR}/proxy/CMIDAPIProxyAbstract.hpp
  ${TESTEE_DIR}/Mocks/MockCMIDAPIProxyAbstract/MockCMIDAPIProxyAbstract.hpp
  ${TESTEE_DIR}/Mocks/MockFileUtilities/MockFileUtilities.hpp
  ${TESTEE_DIR}/Mocks/MockPackageManager/MockPackageManager.hpp
  ${TESTEE_DIR}/Mocks/MockPmCertManager/MockPmCertManager.h
  ${TESTEE_DIR}/Mocks/MockPmCertManager/MockPmCertRetriever.cpp
  ${TESTEE_DIR}/Mocks/MockPmCertManager/MockPmCertRetriever.h
  ${TESTEE_DIR}/Mocks/MockPmPlatformDiscovery/MockPmPlatformDiscovery.hpp
  ${TESTEE_DIR}/Mocks/MockPmPlatformDependencies/MockPmPlatformDependencies.hpp
  ${TESTEE_DIR}/Mocks/MockPmPlatformComponentManager/MockCodesignVerifier.hpp
  ${TESTEE_DIR}/Mocks/MockPmPlatformComponentManager/MockPmPkgUtil.hpp
  ${TESTEE_DIR}/Mocks/MockPmPlatformComponentManager/MockPmPlatformConfiguration.hpp
  ${TESTEE_DIR}/Mocks/MockPmPlatformComponentManager/MockPmPlatformComponentManager.hpp

  # Environment
  ${TESTEE_DIR}/environment/MockEnvironment.h
  ${TESTEE_DIR}/environment/MockEnvironment.cpp
  ${TESTEE_DIR}/environment/UnitTestBase.h
  ${TESTEE_DIR}/environment/UnitTestBase.cpp
)

if(APPLE)
  set_property(TARGET ${component_name} APPEND_STRING PROPERTY COMPILE_FLAGS "-fobjc-arc")
endif()

add_dependencies(
    ${component_name}
    third-party-ciscossl
    third-party-EndpointIdentity
    third-party-gtest
    third-party-jsoncpp
    third-party-PackageManager
    third-party-spdlog
)

if(APPLE)
  add_dependencies(
      ${component_name}
      third-party-ProxyDiscovery-Mac
  )
endif()

target_link_libraries(
  ${component_name}
  $<$<BOOL:${APPLE}>:ProxyDiscovery>
  cmidapi
  crypto
  jsoncpp
  ssl
  util
  xar
  configshared
  "-framework CoreFoundation"
  "-framework IOKit"
  "-framework CoreServices"
  "-framework Security"
  "-framework Foundation"
  "-framework SystemConfiguration"
  ${GTEST_LIBS}
)

target_compile_definitions(
    ${component_name}
    PUBLIC
    "$<$<CONFIG:DEBUG>:DEBUG>"
    "$<$<CONFIG:DEBUG>:CMID_LOG_PATH=\"${CMAKE_CURRENT_BINARY_DIR}\">"
)

target_include_directories(
  ${component_name}
  PRIVATE
      ${CM_THIRDPARTY_EXPORT}/include
      $<$<BOOL:${APPLE}>:${CM_THIRDPARTY_EXPORT}/include/ProxyDiscovery>
      ${cm-client_SOURCE_DIR}
      ${cm-client_SOURCE_DIR}/client/Logger
      ${cm-client_SOURCE_DIR}/OSPackageManager/environment
      ${cm-client_SOURCE_DIR}/OSPackageManager/proxy
      ${cm-client_SOURCE_DIR}/OSPackageManager/macOS
      ${cm-client_SOURCE_DIR}/OSPackageManager/common
      ${cm-client_SOURCE_DIR}/OSPackageManager/configuration
      ${cm-client_SOURCE_DIR}/OSPackageManager/Mocks
      ${cm-client_SOURCE_DIR}/util
)

target_link_directories(
  ${component_name} BEFORE
  PRIVATE
      ${CM_THIRDPARTY_EXPORT}/lib
)

# Add a custom command to copy the binary to the build directory
add_custom_command(
    TARGET ${component_name} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/Resources/mac/signedprocess
            $<TARGET_FILE_DIR:PackageManagerUnitTest>/signedprocess
)

add_custom_command(
    TARGET ${component_name} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/Resources/mac/test_package.pkg
            $<TARGET_FILE_DIR:PackageManagerUnitTest>/test_package.pkg
)

add_custom_command(
    TARGET ${component_name} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/Resources/mac/root
            $<TARGET_FILE_DIR:PackageManagerUnitTest>/root
)

# Create a custom target that depends on the copy command
add_custom_target(CopyBinary ALL DEPENDS ${component_name})


include(GoogleTest)
gtest_discover_tests(${component_name} DISCOVERY_MODE PRE_TEST)
