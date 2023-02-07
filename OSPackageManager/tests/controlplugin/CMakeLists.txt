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
    "$<$<CONFIG:DEBUG>:CMID_DAEMON_PATH=\"${cm-client_SOURCE_DIR}/OSPackageManager/tests/controlplugin/Resources/mac\">"
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

if(CMAKE_GENERATOR STREQUAL "Xcode")
    macro(set_xcode_property TARGET XCODE_PROPERTY XCODE_VALUE)
    set_property (TARGET ${TARGET} PROPERTY XCODE_ATTRIBUTE_${XCODE_PROPERTY}
        ${XCODE_VALUE})
    endmacro (set_xcode_property)

    set_xcode_property(example_test CLANG_CXX_LANGUAGE_STANDARD "gnu++17")
    set_xcode_property(example_test CLANG_CXX_LIBRARY "libc++")
endif()
