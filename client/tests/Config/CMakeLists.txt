# CMakeLists.txt
# Copyright 2022, Cisco Systems, Inc.

add_executable(
  ConfigTest
  ${cm-client_SOURCE_DIR}/client/Configuration/Config.hpp
  ${cm-client_SOURCE_DIR}/client/Configuration/Config.cpp

  # Test files
  ConfigTest.cpp
)

add_dependencies(
    ConfigTest
    third-party-jsoncpp
    third-party-gtest
)

target_link_libraries(
  ConfigTest
  jsoncpp
  ${GTEST_LIBS}
)

target_include_directories(
  ConfigTest
  PRIVATE
      ${CM_THIRDPARTY_EXPORT}/include
      ${cm-client_SOURCE_DIR}/client
)

target_link_directories(
  ConfigTest BEFORE
  PRIVATE
      ${CM_THIRDPARTY_EXPORT}/lib
)

include(GoogleTest)
gtest_discover_tests(ConfigTest)

if(CMAKE_GENERATOR STREQUAL "Xcode")
    macro(set_xcode_property TARGET XCODE_PROPERTY XCODE_VALUE)
    set_property (TARGET ${TARGET} PROPERTY XCODE_ATTRIBUTE_${XCODE_PROPERTY}
        ${XCODE_VALUE})
    endmacro (set_xcode_property)

    set_xcode_property(example_test CLANG_CXX_LANGUAGE_STANDARD "gnu++17")
    set_xcode_property(example_test CLANG_CXX_LIBRARY "libc++")
endif()
