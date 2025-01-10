#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

add_library(test_vectors_headers INTERFACE)
target_include_directories(test_vectors_headers INTERFACE
    ${PROJECT_SOURCE_DIR}
)

include(asn1.cmake)

add_subdirectory(history)
add_subdirectory(safrole)
add_subdirectory(disputes)
add_subdirectory(authorizations)
