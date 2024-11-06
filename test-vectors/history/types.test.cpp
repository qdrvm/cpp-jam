/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <test-vectors/history/vectors.hpp>

GTEST_VECTORS(jam::test_vectors_history::Vectors);

/**
 * Check python generated scale encoding/decoding against test vectors.
 */
GTEST_VECTORS_REENCODE;
