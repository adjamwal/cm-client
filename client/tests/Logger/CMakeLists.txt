# CMakeLists.txt
# Copyright 2022, Cisco Systems, Inc.

set(component_name "LoggerTest")

add_executable(
  ${component_name}
  ${cm-client_SOURCE_DIR}/client/Logger/CMLogger.hpp
  ${cm-client_SOURCE_DIR}/client/Logger/CMLogger.cpp
  ${cm-client_SOURCE_DIR}/ConfigShared/IConfigLogger.hpp

  # Test files
  CMLoggerTest.cpp
)

add_dependencies(
    ${component_name}
    third-party-spdlog
    third-party-gtest
)

target_link_libraries(
  ${component_name}
  configshared
  ${GTEST_LIBS}
  pthread
  $<$<BOOL:${LINUX}>:stdc++fs>
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
