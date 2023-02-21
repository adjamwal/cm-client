# CMakeLists.txt
# Copyright 2023, Cisco Systems, Inc.

cmake_minimum_required(VERSION 3.19)

project(test-executable VERSION 1.0.0)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CXX_EXTENSIONS ON) # -std=gnu++11 instead of -std=c++11.

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(TEST_PACKAGE_MANAGER_SRCS
    Daemon.cpp
    Daemon.hpp
    main.cpp
)

add_executable(cmpackagemanager ${TEST_PACKAGE_MANAGER_SRCS})



