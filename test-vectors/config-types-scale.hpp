/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <optional>
#include <variant>
#include <vector>

#include <scale/scale.hpp>
#include <src/TODO_scale/scale.hpp>

#include <boost/variant.hpp>

#include <test-vectors/config-types.hpp>

namespace scale {
  template <typename T>
  void decodeConfig(ScaleDecoderStream &s, T &v, const auto &config) {
    s >> v;
  }

  template <typename T>
  void decodeConfig(ScaleDecoderStream &s,
                    std::vector<T> &v,
                    const auto &config) {
    size_t size = 0;

    uint8_t b;
    s >> b;
    // fmt::println("b={:02x}  {:08b}", b, b);

    if (b < 0b1000'000) {
      size = b;
    } else {
      // https://github.com/davxy/scalecodec/commit/ec15790aaeb31975c3a171a7d41c4c1a7d16215d#diff-fce22023a6815ef817a56eef11d04889c492696a155425b4edec01cf3feaeb99R52
      if (b == 0xff) {
        // v = int.from_bytes(input.read(8), byteorder = 'little')
        s >> size;
      } else if (b != 0) {
        // Find the first zero bit from the left
        // len = next(i for i in range(8) if (b & (0b1000_0000 >> i)) == 0)
        size_t len = 1;
        for (auto i = 0b1000'0000; i > 0; i = i >> 1) {
          if (b & i) {
            break;
          }
          ++len;
        }

        // buf = self.get_next_bytes(len)
        // v = int.from_bytes(buf, 'little')
        for (auto i = 0; i < len; ++i) {
          uint8_t n = 0;
          s >> n;
          size |= (static_cast<size_t>(n) << (8 * i));
        }

        // Calculate `rem` and combine to get final `v`
        // rem = (b & ((1 << (7 - len)) - 1))
        size_t rem = (b & ((1 << (7 - len)) - 1));

        // v = v + (rem << (8 * len))
        size += (rem << (8 * len));
      }
    }

    v.resize(0);
    v.reserve(size);
    for (size_t i = 0; i < size; ++i) {
      T item;
      decodeConfig(s, item, config);
      v.emplace_back(std::move(item));
    }
  }

  template <typename T>
  void decodeConfig(ScaleDecoderStream &s,
                    std::optional<T> &v,
                    const auto &config) {
    uint8_t i = 0;
    s >> i;
    if (i == 0) {
      v.reset();
    } else if (i == 1) {
      T item;
      decodeConfig(s, item, config);
      v = std::move(item);
    } else {
      raise(DecodeError::WRONG_TYPE_INDEX);
    }
  }

  template <size_t I, typename T, typename... Ts>
  void decodeConfigVariant(size_t i,
                           ScaleDecoderStream &s,
                           auto &v,
                           const auto &config) {
    if (i == I) {
      T item;
      decodeConfig(s, item, config);
      v = std::move(item);
    } else if constexpr (sizeof...(Ts) != 0) {
      decodeConfigVariant<I + 1, Ts...>(i, s, v, config);
    }
  }

  template <typename... Ts>
  void decodeConfig(ScaleDecoderStream &s,
                    boost::variant<Ts...> &v,
                    const auto &config) {
    uint8_t i = 0;
    s >> i;
    if (i >= sizeof...(Ts)) {
      raise(DecodeError::WRONG_TYPE_INDEX);
    }
    decodeConfigVariant<0, Ts...>(i, s, v, config);
  }

  template <typename... Ts>
  void decodeConfig(ScaleDecoderStream &s,
                    std::variant<Ts...> &v,
                    const auto &config) {
    uint8_t i = 0;
    s >> i;
    if (i >= sizeof...(Ts)) {
      raise(DecodeError::WRONG_TYPE_INDEX);
    }
    decodeConfigVariant<0, Ts...>(i, s, v, config);
  }
}  // namespace scale

namespace jam {
  template <typename T, typename ConfigField>
  void decodeConfig(scale::ScaleDecoderStream &s,
                    ConfigVec<T, ConfigField> &v,
                    const auto &config) {
    auto n = v.configSize(config);
    v.v.resize(0);
    v.v.reserve(n);
    for (size_t i = 0; i < n; ++i) {
      T item;
      decodeConfig(s, item, config);
      v.v.emplace_back(std::move(item));
    }
  }

  template <typename T, typename ConfigField>
  scale::ScaleEncoderStream &operator<<(scale::ScaleEncoderStream &s,
                                        const ConfigVec<T, ConfigField> &v) {
    for (auto &item : v.v) {
      s << item;
    }
    return s;
  }
}  // namespace jam
