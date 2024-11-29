/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <test-vectors/history/vectors.hpp>

GTEST_VECTORS(History, history);

/**
 * Check python generated scale encoding/decoding against test vectors.
 */
GTEST_VECTORS_TEST_REENCODE(History, history);
