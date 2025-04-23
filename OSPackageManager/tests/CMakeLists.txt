# CMakeLists.txt
# Copyright 2023, Cisco Systems, Inc.

enable_testing()

add_subdirectory(controlplugin)

if(APPLE)
    add_subdirectory(cmpackagemanager)
elseif(LINUX)
    add_subdirectory(linux)
endif()
