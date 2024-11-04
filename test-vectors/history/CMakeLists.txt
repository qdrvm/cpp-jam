#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

asn1(history history)

add_executable(test_vectors_history_test
  history.test.cpp
)
target_compile_definitions(test_vectors_history_test PRIVATE PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
target_link_libraries(test_vectors_history_test
  fmt::fmt
  headers
  test_vectors_history_types
)
add_test(test_vectors_history_test test_vectors_history_test)
