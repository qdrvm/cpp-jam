#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.25)

option(TESTING "Build and run test suite" ON)
if(TESTING)
    list(APPEND VCPKG_MANIFEST_FEATURES test)
endif()

project(cpp-jam
    VERSION 0.0.1
    LANGUAGES C CXX
)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Python3 REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(libb2 REQUIRED IMPORTED_TARGET GLOBAL libb2)

find_package(Boost CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(jam_crust CONFIG REQUIRED)
find_package(schnorrkel_crust CONFIG REQUIRED)
find_package(scale CONFIG REQUIRED)

add_library(headers INTERFACE)
target_include_directories(headers INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

add_subdirectory(src)

if(TESTING)
    enable_testing()

    find_package(GTest CONFIG REQUIRED)
    set(GTEST_DEPS GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

    add_subdirectory(test-vectors)
endif ()
