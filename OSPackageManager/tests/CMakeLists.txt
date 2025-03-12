# CMakeLists.txt
# Copyright 2023, Cisco Systems, Inc.

enable_testing()

if(APPLE)
    add_subdirectory(cmpackagemanager)
    add_subdirectory(controlplugin)
elseif(LINUX)
    add_subdirectory(linux)
endif()
