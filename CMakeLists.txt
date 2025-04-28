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

option(BUILD_EXAMPLES "Build examples" ON)
option(ASAN "Build with ASAN" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API ON)

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
find_package(Boost.DI CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(jam_crust CONFIG REQUIRED)
find_package(lsquic CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(prometheus-cpp CONFIG REQUIRED)
find_package(qtils CONFIG REQUIRED)
find_package(scale CONFIG REQUIRED)
find_package(schnorrkel_crust CONFIG REQUIRED)
find_package(soralog CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(ZLIB REQUIRED)

include(vcpkg-overlay/cppcodec.cmake)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_compile_options(-fmodules-ts)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
  add_compile_options(-fmodules)
endif()

include_directories(
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/src
  ${CMAKE_BINARY_DIR}/generated
)

add_subdirectory(src)

if (TESTING)
  enable_testing()

  find_package(GTest CONFIG REQUIRED)
  set(GTEST_DEPS GTest::gtest_main)

  add_subdirectory(test-vectors)
  add_subdirectory(tests)
endif ()

if (BUILD_EXAMPLES)
  add_subdirectory(example)
endif ()

function (add_cache_flag var_name flag)
  set(spaced_string " ${${var_name}} ")
  string(FIND "${spaced_string}" " ${flag} " flag_index)
  if (NOT flag_index EQUAL -1)
    return()
  endif ()
  string(COMPARE EQUAL "" "${${var_name}}" is_empty)
  if (is_empty)
    # beautify: avoid extra space at the end if var_name is empty
    set("${var_name}" "${flag}" CACHE STRING "" FORCE)
  else ()
    set("${var_name}" "${flag} ${${var_name}}" CACHE STRING "" FORCE)
  endif ()
endfunction ()
if (ASAN)
  set(ASAN_FLAGS "-fsanitize=address")
  add_cache_flag(CMAKE_CXX_FLAGS ${ASAN_FLAGS})
  add_cache_flag(CMAKE_C_FLAGS ${ASAN_FLAGS})
  add_cache_flag(CMAKE_EXE_LINKER_FLAGS ${ASAN_FLAGS})
  add_cache_flag(CMAKE_SHARED_LINKER_FLAGS ${ASAN_FLAGS})
endif ()
