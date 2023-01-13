# CMakeLists.txt
# Copyright 2022, Cisco Systems, Inc.

add_executable(
  LoggerTest
  ${cm-client_SOURCE_DIR}/client/Logger/CMLogger.hpp
  ${cm-client_SOURCE_DIR}/client/Logger/CMLogger.cpp

  # Test files
  CMLoggerTest.cpp
)

add_dependencies(
    LoggerTest
    third-party-gtest
)

target_link_libraries(
  LoggerTest
  ${GTEST_LIBS}
)

target_include_directories(
  LoggerTest
  PRIVATE
      ${CM_THIRDPARTY_EXPORT}/include
      ${cm-client_SOURCE_DIR}/client
)

target_link_directories(
  LoggerTest BEFORE
  PRIVATE
      ${CM_THIRDPARTY_EXPORT}/lib
)

include(GoogleTest)
gtest_discover_tests(LoggerTest)

if(CMAKE_GENERATOR STREQUAL "Xcode")
    macro(set_xcode_property TARGET XCODE_PROPERTY XCODE_VALUE)
    set_property (TARGET ${TARGET} PROPERTY XCODE_ATTRIBUTE_${XCODE_PROPERTY}
        ${XCODE_VALUE})
    endmacro (set_xcode_property)

    set_xcode_property(LoggerTest CLANG_CXX_LANGUAGE_STANDARD "gnu++17")
    set_xcode_property(LoggerTest CLANG_CXX_LIBRARY "libc++")
endif()
