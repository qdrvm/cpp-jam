#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.25)

project(cpp-jam
    VERSION 0.0.1
    LANGUAGES C CXX
)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(TESTING "Build and run test suite" ON)

find_package(Python3 REQUIRED)

find_package(Boost CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(scale CONFIG REQUIRED)

add_subdirectory(src)

if(TESTING)
    enable_testing()
    add_subdirectory(test-vectors)
endif ()
