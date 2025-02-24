# CMakeLists.txt
# Copyright 2023, Cisco Systems, Inc.

set(component_name "ControlPluginTest")

add_executable(${component_name}
  ${cm-client_SOURCE_DIR}/client/Logger/CMLogger.cpp
  # Test files
  ModuleControlPluginTest.cpp
  PmAgentControllerTest.cpp
  FakeProcessWrapper.cpp
  FakeProcessWrapper.hpp
)

add_dependencies(
    ${component_name}
    third-party-gtest
)

target_link_libraries(
  ${component_name}
  pmcontrolplugin
  configshared
  ${GTEST_LIBS}
)

target_compile_definitions(
    ${component_name}
    PUBLIC
    "$<$<CONFIG:DEBUG>:DEBUG>"
    "$<$<CONFIG:DEBUG>:CM_CONFIG_PATH=\"${CMAKE_CURRENT_BINARY_DIR}\">"
    "$<$<CONFIG:DEBUG>:CMID_DAEMON_PATH=\"${cm-client_SOURCE_DIR}/OSPackageManager/tests/controlplugin/Resources/mac\">"
    "$<$<CONFIG:DEBUG>:CM_SHARED_LOG_PATH=\"${CMAKE_CURRENT_BINARY_DIR}\">"
    "$<$<CONFIG:DEBUG>:CMID_LOG_PATH=\"${CMAKE_CURRENT_BINARY_DIR}\">"
)

target_include_directories(
  ${component_name}
  PRIVATE
      ${CM_THIRDPARTY_EXPORT}/include
      ${cm-client_SOURCE_DIR}/client/Logger
      ${cm-client_SOURCE_DIR}/OSPackageManager/controlplugin
)

target_link_directories(
  ${component_name} BEFORE
  PRIVATE
      ${CM_THIRDPARTY_EXPORT}/lib
)

include(GoogleTest)
gtest_discover_tests(${component_name} DISCOVERY_MODE PRE_TEST)

