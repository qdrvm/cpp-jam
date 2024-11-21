/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <test-vectors/history/history.hpp>
#include <test-vectors/history/types.diff.hpp>
#include <test-vectors/history/vectors.hpp>

GTEST_VECTORS(History, jam::test_vectors_history::Vectors);

GTEST_VECTORS_TEST_TRANSITION(History, jam::history);
