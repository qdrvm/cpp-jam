/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <test-vectors/safrole/vectors.hpp>

using jam::test_vectors::test_reencode;
using jam::test_vectors_safrole::Vectors;

int main() {
  test_reencode<Vectors<false>>();
  test_reencode<Vectors<true>>();
  fmt::println("ok");
  return 0;
}
