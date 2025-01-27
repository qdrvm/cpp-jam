#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.25)

option(TESTING "Build and run test suite" ON)
if (TESTING)
  list(APPEND VCPKG_MANIFEST_FEATURES test)
endif ()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(cpp-jam
    VERSION 0.0.1
    LANGUAGES CXX
)

message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "Boost_DIR: ${Boost_DIR}")

find_package(Python3 REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(libb2 REQUIRED IMPORTED_TARGET GLOBAL libb2)

find_package(Boost CONFIG REQUIRED COMPONENTS algorithm outcome program_options)
find_package(fmt CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(jam_crust CONFIG REQUIRED)
find_package(scale CONFIG REQUIRED)
find_package(soralog CONFIG REQUIRED)
find_package(schnorrkel_crust CONFIG REQUIRED)
find_package(Boost.DI CONFIG REQUIRED)
find_package(qtils CONFIG REQUIRED)
find_package(prometheus-cpp CONFIG REQUIRED)

add_library(headers INTERFACE)
target_include_directories(headers INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src_>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

add_subdirectory(src_)

add_subdirectory(src)

if (TESTING)
  enable_testing()

  find_package(GTest CONFIG REQUIRED)
  set(GTEST_DEPS GTest::gtest_main)

  add_subdirectory(test-vectors)
  add_subdirectory(tests)
endif ()
