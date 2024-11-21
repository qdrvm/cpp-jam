/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <test-vectors/safrole/vectors.hpp>

GTEST_VECTORS(jam::test_vectors_safrole::Vectors);

/**
 * Check python generated scale encoding/decoding against test vectors.
 */
GTEST_VECTORS_REENCODE;
