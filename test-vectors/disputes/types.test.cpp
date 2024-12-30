/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <test-vectors/disputes/vectors.hpp>

GTEST_VECTORS(Disputes, disputes);

/**
 * Check python generated scale encoding/decoding against test vectors.
 */
GTEST_VECTORS_TEST_REENCODE(Disputes, disputes);
