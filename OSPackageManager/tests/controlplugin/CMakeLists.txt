# CMakeLists.txt
# Copyright 2023, Cisco Systems, Inc.

add_executable(
  ControlPluginTest
  ${cm-client_SOURCE_DIR}/client/Logger/CMLogger.cpp
  # Test files
  ModuleControlPluginTest.cpp
  PmAgentControllerTest.cpp
)

add_dependencies(
    ControlPluginTest
    third-party-gtest
)

target_link_libraries(
  ControlPluginTest
  pmcontrolplugin
  ${GTEST_LIBS}
)

target_compile_definitions(
    ControlPluginTest
    PUBLIC
    "$<$<CONFIG:DEBUG>:DEBUG>"
    "$<$<CONFIG:DEBUG>:CM_CONFIG_PATH=\"${CMAKE_CURRENT_BINARY_DIR}\">"
    "$<$<CONFIG:DEBUG>:CMID_DAEMON_PATH=\"${CMAKE_BINARY_DIR}/OSPackageManager\">"
    "$<$<CONFIG:DEBUG>:CM_SHARED_LOG_PATH=\"${CMAKE_CURRENT_BINARY_DIR}\">"
    "$<$<CONFIG:DEBUG>:CMID_LOG_PATH=\"${CMAKE_CURRENT_BINARY_DIR}\">"
)

target_include_directories(
  ControlPluginTest
  PRIVATE
      ${CM_THIRDPARTY_EXPORT}/include
      ${cm-client_SOURCE_DIR}/client/Logger
      ${cm-client_SOURCE_DIR}/OSPackageManager/controlplugin
)

target_link_directories(
  ControlPluginTest BEFORE
  PRIVATE
      ${CM_THIRDPARTY_EXPORT}/lib
)

include(GoogleTest)
gtest_discover_tests(ControlPluginTest)

