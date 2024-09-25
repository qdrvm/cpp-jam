/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include "vectors.hpp"

/**
 * Check python generated scale encoding/decoding against test vectors.
 */
GTEST_VECTORS {
  auto expected = Vectors::readRaw(path);
  auto decoded = Vectors::decode(expected);
  auto reencoded = scale::encode(*decoded).value();
  EXPECT_EQ(reencoded, expected);
}
