/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "vectors.hpp"

using jam::test_vectors_safrole::Vectors;

/**
 * Check python generated scale encoding/decoding against test vectors.
 */
template <bool full>
void test_reencode() {
  Vectors<full> vectors;
  for (auto &path : vectors.paths) {
    fmt::println("{}", path.native());
    auto expected = vectors.readRaw(path);
    auto decoded = vectors.decode(expected);
    auto reencoded = scale::encode(*decoded).value();
    if (reencoded != expected) {
      throw std::logic_error{"reencoded"};
    }
  }
}

int main() {
  test_reencode<false>();
  test_reencode<true>();
  fmt::println("ok");
  return 0;
}
