#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.25)

option(MORUM_BUILD_TESTS "Build tests" ON)
option(MORUM_BUILD_BENCHMARKS "Build benchmarks" ON)
option(MORUM_BUILD_TRACY "Enable Tracy profiler" OFF)

if (MORUM_BUILD_TESTS)
    list(APPEND VCPKG_MANIFEST_FEATURES "test")
endif()
if (MORUM_BUILD_BENCHMARKS)
    list(APPEND VCPKG_MANIFEST_FEATURES "benchmark")
endif()
if (MORUM_BUILD_TRACY)
    list(APPEND VCPKG_MANIFEST_FEATURES "tracy")
endif()

project(morum
    VERSION 0.1.0
    LANGUAGES C CXX
)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_compile_options(-Wall -Wextra)

option(MORUM_TRACE "Enable tracing" OFF)

find_package(Python3 REQUIRED)
find_package(Boost CONFIG REQUIRED)
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


if (MORUM_ASAN)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()

if (MORUM_UBSAN)
    add_compile_options(-fsanitize=undefined -fno-sanitize-recovery=undefined)
    add_link_options(-fsanitize=undefined)
endif()
if (MORUM_TSAN)
    add_compile_options(-fsanitize=thread)
    add_link_options(-fsanitize=thread)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    option(QTILS_ASSERT "Enable asserts" OFF)
else()
    option(QTILS_ASSERT "Enable asserts" ON)
endif()


add_subdirectory(src)

if(MORUM_BUILD_BENCHMARKS)
    find_package(benchmark CONFIG REQUIRED)
    add_subdirectory(benchmark)
endif()

if(MORUM_BUILD_TESTS)
    find_package(GTest CONFIG REQUIRED)
    set(GTEST_DEPS GTest::gtest_main)
  
    enable_testing()
    
    function (morum_add_test TEST_NAME TEST_SRC)
        add_executable(${TEST_NAME} ${TEST_SRC})
        target_include_directories(${TEST_NAME} 
            PUBLIC 
            ${CMAKE_SOURCE_DIR}/include
        )
        add_compile_definitions(${TEST_NAME} PUBLIC MORUM_TEST_BUILD)
        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME} WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/test_bin")
        set_target_properties(${TEST_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test_bin")
    endfunction()

    add_subdirectory(test-vectors)
    add_subdirectory(test)
endif ()
