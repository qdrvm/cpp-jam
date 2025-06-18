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
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API ON)

add_compile_options(-Wall -Wextra)

if ((CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo") AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-flto=thin)
endif()

if(DEFINED CMAKE_TOOLCHAIN_FILE AND CMAKE_TOOLCHAIN_FILE MATCHES "vcpkg")
    if(DEFINED VCPKG_TARGET_TRIPLET AND VCPKG_TARGET_TRIPLET)
        set(DETECTED_TRIPLET ${VCPKG_TARGET_TRIPLET})
        message(STATUS "Using vcpkg triplet from VCPKG_TARGET_TRIPLET: ${DETECTED_TRIPLET}")
    else()
        if(WIN32)
            if(CMAKE_SIZEOF_VOID_P EQUAL 8)
                set(DETECTED_TRIPLET "x64-windows")
            else()
                set(DETECTED_TRIPLET "x86-windows")
            endif()
        elseif(APPLE)
            if(CMAKE_SIZEOF_VOID_P EQUAL 8)
                set(DETECTED_TRIPLET "x64-osx")
            else()
                set(DETECTED_TRIPLET "arm64-osx")
            endif()
        else()
            if(CMAKE_SIZEOF_VOID_P EQUAL 8)
                set(DETECTED_TRIPLET "x64-linux")
            else()
                set(DETECTED_TRIPLET "x86-linux")
            endif()
        endif()
        message(STATUS "Auto-detected vcpkg triplet: ${DETECTED_TRIPLET}")
    endif()
    
    set(CMAKE_INSTALL_LIBDIR "${DETECTED_TRIPLET}/lib")
    set(CMAKE_INSTALL_INCLUDEDIR "${DETECTED_TRIPLET}/include")
    
    message(STATUS "CMAKE_INSTALL_LIBDIR: ${CMAKE_INSTALL_LIBDIR}")
    message(STATUS "CMAKE_INSTALL_INCLUDEDIR: ${CMAKE_INSTALL_INCLUDEDIR}")
endif()

include(GNUInstallDirs)

message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "Boost_DIR: ${Boost_DIR}")

option(MORUM_ASAN "Enable address sanitizer" OFF)
option(MORUM_TSAN "Enable address sanitizer" OFF)
option(MORUM_UBSAN "Enable address sanitizer" OFF)
option(MORUM_TRACE "Enable tracing" OFF)

set(Python3_FIND_VIRTUALENV ONLY)
find_package(Python3 REQUIRED)
find_package(Boost CONFIG REQUIRED COMPONENTS algorithm program_options outcome)
find_package(fmt CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(scale CONFIG REQUIRED)
find_package(soralog CONFIG REQUIRED)
find_package(Boost.DI CONFIG REQUIRED)
find_package(qtils CONFIG REQUIRED)
find_package(prometheus-cpp CONFIG REQUIRED)
find_package(RocksDB CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(qdrvm-crates CONFIG REQUIRED)

find_library(BLAKE2_LIB blake2b)
add_library(blake2b STATIC IMPORTED)
set_property(TARGET blake2b PROPERTY IMPORTED_LOCATION ${BLAKE2_LIB})

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_compile_options(-fmodules-ts)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
  add_compile_options(-fmodules)
endif()

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
