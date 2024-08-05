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

option(TESTING "Build and run test suite" ON)

add_subdirectory(src)

if(TESTING)
    add_subdirectory(test-vectors)
endif ()
