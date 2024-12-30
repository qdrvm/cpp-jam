/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <test-vectors/safrole/vectors.hpp>

GTEST_VECTORS(Safrole, safrole);

/**
 * Check python generated scale encoding/decoding against test vectors.
 */
GTEST_VECTORS_TEST_REENCODE(Safrole, safrole);
