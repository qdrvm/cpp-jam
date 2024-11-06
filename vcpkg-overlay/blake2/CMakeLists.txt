cmake_minimum_required(VERSION 3.28)
project(blake2 VERSION 1.0.0 LANGUAGES C)

include(GNUInstallDirs)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL x86_64)
    set(VECTOR_EXT sse)
elseif (CMAKE_SYSTEM_PROCESSOR STREQUAL arm64)
    set(VECTOR_EXT neon)
else()
    message(AUTHOR_WARNING "Unexpected architecture ${CMAKE_SYSTEM_PROCESSOR}, will use unoptimized implementation.")
    set(VECTOR_EXT ref)
endif()

add_library(blake2b ${VECTOR_EXT}/blake2b.c)
target_compile_options(blake2b PRIVATE -O3 -Wall -Wextra -std=c89 -pedantic -Wno-long-long)

install(TARGETS blake2b)
install(FILES "${CMAKE_SOURCE_DIR}/${VECTOR_EXT}/blake2.h" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
