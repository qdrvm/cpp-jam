/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jam/keccak.hpp>

/**
 * Common functions used in tests
 */

namespace jam {
  // [GP 0.4.3 3.7.2]
  // https://github.com/gavofyork/graypaper/blob/v0.4.3/text/notation.tex#L132
  template <size_t X, size_t Y>
  qtils::BytesN<X + Y> frown(
      const qtils::BytesN<X> &x, const qtils::BytesN<Y> &y) {
    qtils::BytesN<X + Y> xy;
    memcpy(xy.data(), x.data(), X);
    memcpy(xy.data() + X, y.data(), Y);
    return xy;
  }

  // [GP 0.4.3 3.8.1]
  // https://github.com/gavofyork/graypaper/blob/v0.4.3/text/notation.tex#L153
  inline auto mathcal_H_K(qtils::BytesIn m) {
    return Keccak::hash(m);
  }
}  // namespace jam
