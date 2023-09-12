# CMakeLists.txt
# Copyright 2022, Cisco Systems, Inc.

add_executable(
  PmConfigTest
  ${cm-client_SOURCE_DIR}/configshared/Config.cpp
  ${cm-client_SOURCE_DIR}/OSPackageManager/MacOS/PMLogger.cpp
  # Test files
  ConfigTest.cpp
)

add_dependencies(
    PmConfigTest
    third-party-spdlog
    third-party-jsoncpp
    third-party-gtest
    third-party-ProxyDiscovery-Mac
)

target_link_libraries(
  PmConfigTest
  jsoncpp
  ProxyDiscovery
  ${GTEST_LIBS}
)

target_compile_definitions(
    PmConfigTest
    PUBLIC
    "$<$<CONFIG:DEBUG>:DEBUG>"
    "$<$<CONFIG:DEBUG>:CM_CONFIG_PATH=\"${CMAKE_CURRENT_BINARY_DIR}\">"
    "$<$<CONFIG:DEBUG>:CMID_DAEMON_PATH=\"${CM_THIRDPARTY_EXPORT}/bin\">"
    "$<$<CONFIG:DEBUG>:CM_SHARED_LOG_PATH=\"${CMAKE_CURRENT_BINARY_DIR}\">"
    "$<$<CONFIG:DEBUG>:CMID_LOG_PATH=\"${CMAKE_CURRENT_BINARY_DIR}\">"
)

target_include_directories(
  PmConfigTest
  PRIVATE
      ${CM_THIRDPARTY_EXPORT}/include
      ${cm-client_SOURCE_DIR}
      ${cm-client_SOURCE_DIR}/OSPackageManager/MacOS
      ${cm-client_SOURCE_DIR}/configshared
)

target_link_directories(
  PmConfigTest BEFORE
  PRIVATE
      ${CM_THIRDPARTY_EXPORT}/lib
  ${inherited}
)

include(GoogleTest)
gtest_discover_tests(PmConfigTest DISCOVERY_MODE PRE_TEST)

if(CMAKE_GENERATOR STREQUAL "Xcode")
    macro(set_xcode_property TARGET XCODE_PROPERTY XCODE_VALUE)
    set_property (TARGET ${TARGET} PROPERTY XCODE_ATTRIBUTE_${XCODE_PROPERTY}
        ${XCODE_VALUE})
    endmacro (set_xcode_property)

    set_xcode_property(PmConfigTest CLANG_CXX_LANGUAGE_STANDARD "gnu++17")
    set_xcode_property(PmConfigTest CLANG_CXX_LIBRARY "libc++")
endif()
