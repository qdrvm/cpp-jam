/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstring>
#include <stdexcept>

#include <boost/endian/conversion.hpp>
#include <crypto/blake.hpp>
#include <crypto/keccak.hpp>
#include <qtils/byte_arr.hpp>

/**
 * Common functions used in tests
 */

namespace jam {
  template <size_t N>
  qtils::ByteArr<N> first_bytes(qtils::BytesIn bytes) {
    if (bytes.size() < N) {
      throw std::logic_error("first_bytes");
    }
    qtils::ByteArr<N> first;
    memcpy(first.data(), bytes.data(), N);
    return first;
  }

  // [GP 0.4.5 3.7]
  // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/notation.tex#L116
  auto &circlearrowleft(const auto &s, auto i) {
    return s[i % s.size()];
  }

  // [GP 0.4.5 3.7.2]
  // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/notation.tex#L132
  template <size_t X, size_t Y>
  qtils::ByteArr<X + Y> frown(const qtils::ByteArr<X> &x, const qtils::ByteArr<Y> &y) {
    qtils::ByteArr<X + Y> xy;
    memcpy(xy.data(), x.data(), X);
    memcpy(xy.data() + X, y.data(), Y);
    return xy;
  }

  // [GP 0.4.5 3.7.2]
  // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/notation.tex#L132
  template <size_t X>
  qtils::ByteArr<X + 1> doubleplus(const qtils::ByteArr<X> &x, uint8_t i) {
    qtils::ByteArr<1> y{i};
    return frown(x, y);
  }

  // [GP 0.4.5 C.1.2 300]
  // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/serialization.tex#L29
  template <size_t N>
  qtils::ByteArr<N> mathcal_E(uint64_t x) {
    qtils::ByteArr<N> out;
    boost::endian::endian_store<uint64_t, N, boost::endian::order::little>(
        out.data(), x);
    return out;
  }

  // [GP 0.4.5 C.1.2 300]
  // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/serialization.tex#L29
  template <size_t N>
  auto de(qtils::ByteArr<N> x) {
    return boost::endian::
        endian_load<uint64_t, N, boost::endian::order::little>(x.data());
  }

  // [GP 0.4.5 3.8.1]
  // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/notation.tex#L153
  inline auto mathcal_H(qtils::BytesIn m) {
    return crypto::Blake::hash(m);
  }

  // [GP 0.4.5 3.8.1]
  // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/notation.tex#L153
  inline auto mathcal_H_K(qtils::BytesIn m) {
    return crypto::Keccak::hash(m);
  }
}  // namespace jam
