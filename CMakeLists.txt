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

option(MORUM_TRACE "Enable tracing" OFF)

find_package(Python3 REQUIRED)
find_package(Boost CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(scale CONFIG REQUIRED)
find_package(RocksDB CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Tracy CONFIG REQUIRED)
find_package(qtils CONFIG REQUIRED)

find_library(BLAKE2_LIB blake2b)
add_library(blake2b STATIC IMPORTED)
set_property(TARGET blake2b PROPERTY IMPORTED_LOCATION ${BLAKE2_LIB})

add_compile_options(-Werror -Wall -Wextra -pedantic)

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
if (MORUM_MSAN)
    add_compile_options(-fsanitize=memory)
    add_link_options(-fsanitize=memory)
endif()

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

add_subdirectory(src)

if(MORUM_BUILD_BENCHMARKS)
    find_package(benchmark CONFIG REQUIRED)
    add_subdirectory(benchmark)
endif()

if(MORUM_BUILD_TESTS)
    enable_testing()

    find_package(GTest CONFIG REQUIRED)
    #add_subdirectory(test-vectors)
    add_subdirectory(test)
endif ()
