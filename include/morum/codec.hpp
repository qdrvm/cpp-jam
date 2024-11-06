/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <concepts>
#include <cstdint>
#include <tuple>
#include <vector>
#include <span>

#include <morum/common.hpp>

namespace morum {

  template <typename T>
  concept OutStream = true;

  template <typename T>
  concept InStream = true;

  template <typename T>
  concept Encodable = true;

  struct VectorStream {
    void append(uint8_t b) {
      data.push_back(b);
    }

    std::vector<uint8_t> data;
  };

  template<size_t N>
  struct ArrayStream {
    void append(uint8_t b) {
      assert(current < N);
      data[current] = b;
      current++;
    }

    std::array<uint8_t, N> data;
    size_t current = 0;
  };

  struct SpanStream {
    uint8_t read_byte() {
      assert(current < data.size_bytes());
      return data[current++];
    }

    std::span<uint8_t> data;
    size_t current = 0;
  };


  template <OutStream S, Encodable... Ts>
    requires(sizeof...(Ts) > 1)
  void encode(S &stream, const Ts &...tuple) {
    (encode(stream, tuple), ...);
  }

  template <OutStream S, Encodable... Ts>
  consteval void encode(S &stream, const std::tuple<Ts...> &tuple) {
    for (int i = 0; i < tuple.size(); i++) {
      encode(stream, std::get<i>(tuple));
    }
  }

  template <OutStream S, std::unsigned_integral I>
  void encode_integer_fixed(S &stream, I integer, size_t len) {
    for (size_t i = 0; i < len; i++) {
      stream.append(integer % 256);
      integer /= 256;
    }
  }

  template <InStream S, std::unsigned_integral I>
  void decode_integer_fixed(S &stream, I& integer, size_t len) {
    integer = 0;
    for (size_t i = 0; i < len; i++) {
      uint8_t byte = stream.read_byte();
      integer <<= 8;
      integer |= static_cast<I>(byte);
    }
  }

  template <OutStream S, std::unsigned_integral I>
  void encode_integer_29bit(S &stream, I integer) {
    QTILS_ASSERT(integer < (1 << 29));

    if (integer >= (1ul << 21)) {
      stream.append(256 - 32 + integer / (1ul << 24));
      encode_integer_fixed(stream, integer % (1 << 24), 3);
    }
    int l = 3;
    if (integer == 0) {
      l = 0;
    } else {
      auto power = 1ul << 21;
      while (integer < power) {
        l--;
        power >>= 8;
      }
    }
    stream.append(256 - (1 << (8 - l)) + integer / (1ul << (8 * l)));
    encode_integer_fixed(stream, integer % (1ul << (8 * l)), l);
  }

  template <OutStream S, std::unsigned_integral I>
  void encode(S &stream, I integer) {
    if (integer >= (1ul << 56)) {
      stream.append(255);
      encode_integer_fixed(stream, integer, 8);
    }
    int l = 8;
    if (integer == 0) {
      l = 0;
    } else {
      auto power = 1ul << 56;
      while (integer < power) {
        l--;
        power >>= 8;
      }
    }
    stream.append(256 - (1 << (8 - l)) + integer / (1ul << (8 * l)));
    encode_integer_fixed(stream, integer % (1ul << (8 * l)), l);
  }
}  // namespace morum
