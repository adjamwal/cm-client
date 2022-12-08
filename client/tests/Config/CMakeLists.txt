# CMakeLists.txt
# Copyright 2022, Cisco Systems, Inc.

#
#  Not an actual test -- meant as a template / example
#

add_executable(
  example_test
  example_test.cpp
)

add_dependencies(
    example_test
    third-party-gtest
)

target_link_libraries(
  example_test
  ${GTEST_LIBS}
)

target_include_directories(
  example_test
  PRIVATE
      ${CM_THIRDPARTY_EXPORT}/include
)

target_link_directories(
  example_test BEFORE
  PRIVATE
      ${CM_THIRDPARTY_EXPORT}/lib
)

include(GoogleTest)
gtest_discover_tests(example_test)

if(CMAKE_GENERATOR STREQUAL "Xcode")
    macro(set_xcode_property TARGET XCODE_PROPERTY XCODE_VALUE)
    set_property (TARGET ${TARGET} PROPERTY XCODE_ATTRIBUTE_${XCODE_PROPERTY}
        ${XCODE_VALUE})
    endmacro (set_xcode_property)

    set_xcode_property(example_test CLANG_CXX_LANGUAGE_STANDARD "gnu++17")
    set_xcode_property(example_test CLANG_CXX_LIBRARY "libc++")
endif()
