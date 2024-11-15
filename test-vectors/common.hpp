/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdexcept>

#include <boost/endian/conversion.hpp>

#include <jam/blake.hpp>
#include <jam/keccak.hpp>

/**
 * Common functions used in tests
 */

namespace jam {
  template <size_t N>
  qtils::BytesN<N> first_bytes(qtils::BytesIn bytes) {
    if (bytes.size() < N) {
      throw std::logic_error("first_bytes");
    }
    qtils::BytesN<N> first;
    memcpy(first.data(), bytes.data(), N);
    return first;
  }

  // [GP 0.3.6 3.7]
  auto &circlearrowleft(const auto &s, auto i) {
    return s[i % s.size()];
  }

  // [GP 0.4.5 3.7.2]
  // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/notation.tex#L132
  template <size_t X, size_t Y>
  qtils::BytesN<X + Y> frown(
      const qtils::BytesN<X> &x, const qtils::BytesN<Y> &y) {
    qtils::BytesN<X + Y> xy;
    memcpy(xy.data(), x.data(), X);
    memcpy(xy.data() + X, y.data(), Y);
    return xy;
  }

  // [GP 0.3.6 3.7.2]
  template <size_t X>
  qtils::BytesN<X + 1> doubleplus(const qtils::BytesN<X> &x, uint8_t i) {
    return frown(x, std::array{i});
  }

  // [GP 0.3.6 C.1.2 271]
  template <size_t N>
  qtils::BytesN<N> mathcal_E(uint64_t x) {
    qtils::BytesN<N> out;
    boost::endian::endian_store<uint64_t, N, boost::endian::order::little>(
        out.data(), x);
    return out;
  }

  // [GP 0.3.6 C.1.2 271]
  template <size_t N>
  auto de(qtils::BytesN<N> x) {
    return boost::endian::
        endian_load<uint64_t, N, boost::endian::order::little>(x.data());
  }

  // [GP 0.3.6 3.8.1]
  inline auto mathcal_H(qtils::BytesIn m) {
    return Blake::hash(m);
  }

  // [GP 0.4.5 3.8.1]
  // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/notation.tex#L153
  inline auto mathcal_H_K(qtils::BytesIn m) {
    return Keccak::hash(m);
  }
}  // namespace jam
