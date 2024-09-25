/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "vectors.hpp"

using jam::test_vectors::test_reencode;
using jam::test_vectors_history::Vectors;

int main() {
  test_reencode<Vectors>();
  fmt::println("ok");
  return 0;
}
