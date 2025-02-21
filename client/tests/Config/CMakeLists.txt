# CMakeLists.txt
# Copyright 2022, Cisco Systems, Inc.

set(component_name "ConfigTest")

add_executable(
  ${component_name}
  ${cm-client_SOURCE_DIR}/client/Logger/CMLogger.cpp
  # Test files
  ConfigTest.cpp
)

add_dependencies(
    ${component_name}
    third-party-spdlog
    third-party-jsoncpp
    third-party-gtest
)

target_link_libraries(
  ${component_name}
  jsoncpp
  configshared
  ${GTEST_LIBS}
  pthread
  $<$<BOOL:${LINUX}>:stdc++fs>
)

target_compile_definitions(
    ${component_name}
    PUBLIC
    "$<$<CONFIG:DEBUG>:DEBUG>"
    "$<$<CONFIG:DEBUG>:CM_CONFIG_PATH=\"${CMAKE_CURRENT_BINARY_DIR}\">"
    "$<$<CONFIG:DEBUG>:CMID_DAEMON_PATH=\"${CM_THIRDPARTY_EXPORT}/bin\">"
    "$<$<CONFIG:DEBUG>:CM_SHARED_LOG_PATH=\"${CMAKE_CURRENT_BINARY_DIR}\">"
    "$<$<CONFIG:DEBUG>:CMID_LOG_PATH=\"${CMAKE_CURRENT_BINARY_DIR}\">"
)

target_include_directories(
  ${component_name}
  PRIVATE
      ${CM_THIRDPARTY_EXPORT}/include
      ${cm-client_SOURCE_DIR}/client
)

target_link_directories(
  ${component_name} BEFORE
  PRIVATE
      ${CM_THIRDPARTY_EXPORT}/lib
      ${inherited}
)

include(GoogleTest)
gtest_discover_tests(${component_name} DISCOVERY_MODE PRE_TEST)

if(CMAKE_GENERATOR STREQUAL "Xcode")
    macro(set_xcode_property TARGET XCODE_PROPERTY XCODE_VALUE)
    set_property (TARGET ${TARGET} PROPERTY XCODE_ATTRIBUTE_${XCODE_PROPERTY}
        ${XCODE_VALUE})
    endmacro (set_xcode_property)

    set_xcode_property(${component_name} CLANG_CXX_LANGUAGE_STANDARD "gnu++17")
    set_xcode_property(${component_name} CLANG_CXX_LIBRARY "libc++")
endif()
