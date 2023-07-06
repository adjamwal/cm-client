# CMakeLists.txt
# Copyright 2023, Cisco Systems, Inc.

get_filename_component(TESTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}" DIRECTORY)
get_filename_component(TESTEE_DIR "${TESTS_DIR}" DIRECTORY)

file(GLOB_RECURSE TEST_SRCS Test*.cpp)

add_executable(
  PackageManagerUnitTest
  ${cm-client_SOURCE_DIR}/client/Logger/CMLogger.cpp

  # Test files
  ${TEST_SRCS}

  # Dependencies
  ${TESTEE_DIR}/macOS/PmPkgUtilWrapper.cpp
  ${TESTEE_DIR}/macOS/PmPlatformConfiguration.cpp
  ${TESTEE_DIR}/macOS/PmCertManager.cpp
  ${TESTEE_DIR}/macOS/PmPlatformComponentManager.cpp
  ${TESTEE_DIR}/macOS/PmPlatformDiscovery.cpp
  ${TESTEE_DIR}/macOS/PmLogger.cpp
  ${TESTEE_DIR}/macOS/PmLogger.hpp
  
  # Mocks
  ${TESTEE_DIR}/proxy/CMIDAPIProxy.hpp
  ${TESTEE_DIR}/proxy/CMIDAPIProxyAbstract.hpp
  ${TESTEE_DIR}/Mocks/MockPmCertManager/MockPmCertManager.h
  ${TESTEE_DIR}/Mocks/MockPmCertManager/MockPmCertRetriever.cpp
  ${TESTEE_DIR}/Mocks/MockPmCertManager/MockPmCertRetriever.h
)

add_dependencies(
    PackageManagerUnitTest
    third-party-ciscossl
    third-party-EndpointIdentity
    third-party-gtest
    third-party-PackageManager
    third-party-spdlog
)

target_link_libraries(
  PackageManagerUnitTest
  cmidapi
  crypto
  ssl
  ${GTEST_LIBS}
)

target_compile_definitions(
    PackageManagerUnitTest
    PUBLIC
    "$<$<CONFIG:DEBUG>:DEBUG>"
    "$<$<CONFIG:DEBUG>:CMID_LOG_PATH=\"${CMAKE_CURRENT_BINARY_DIR}\">"
)

target_include_directories(
  PackageManagerUnitTest
  PRIVATE
      ${CM_THIRDPARTY_EXPORT}/include
      ${cm-client_SOURCE_DIR}/client/Logger
      ${cm-client_SOURCE_DIR}/OSPackageManager/macOS
      ${cm-client_SOURCE_DIR}/OSPackageManager/Mocks
)

target_link_directories(
  PackageManagerUnitTest BEFORE
  PRIVATE
      ${CM_THIRDPARTY_EXPORT}/lib
)

include(GoogleTest)
gtest_discover_tests(PackageManagerUnitTest DISCOVERY_MODE PRE_TEST)
