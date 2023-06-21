# CMakeLists.txt
# Copyright 2023, Cisco Systems, Inc.

get_filename_component(TESTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}" DIRECTORY)
get_filename_component(TESTEE_DIR "${TESTS_DIR}" DIRECTORY)

add_executable(
  PackageManagerUnitTest
  ${cm-client_SOURCE_DIR}/client/Logger/CMLogger.cpp
  # Test files
  CMPackageManagerUnitTest.cpp
  ${TESTEE_DIR}/macOS/PmPlatformConfiguration.cpp
)

add_dependencies(
    PackageManagerUnitTest
    third-party-ciscossl
    third-party-EndpointIdentity
    third-party-gtest
    third-party-PackageManager
)

target_link_libraries(
  PackageManagerUnitTest
  cmidapi
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
)

target_link_directories(
  PackageManagerUnitTest BEFORE
  PRIVATE
      ${CM_THIRDPARTY_EXPORT}/lib
)

include(GoogleTest)
gtest_discover_tests(PackageManagerUnitTest DISCOVERY_MODE PRE_TEST)
