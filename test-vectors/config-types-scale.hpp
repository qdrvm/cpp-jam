/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <array>
#include <optional>
#include <variant>
#include <vector>

#include <scale/scale.hpp>
#include <src/TODO_scale/scale.hpp>
#include <src/jam/empty.hpp>

#include <boost/variant.hpp>
#include <qtils/bytes.hpp>

#include <test-vectors/config-types.hpp>

namespace jam {

  size_t decodeJamCompactInteger(scale::ScaleDecoderStream &s) {
    size_t value = 0;

    uint8_t b;
    s >> b;
    // fmt::println("b={:02x}  {:08b}", b, b);

    if (b < 0b1000'000) {
      value = b;
    } else {
      // https://github.com/davxy/scalecodec/commit/ec15790aaeb31975c3a171a7d41c4c1a7d16215d#diff-fce22023a6815ef817a56eef11d04889c492696a155425b4edec01cf3feaeb99R52
      if (b == 0xff) {
        // v = int.from_bytes(input.read(8), byteorder = 'little')
        s >> value;
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
          value |= (static_cast<size_t>(n) << (8 * i));
        }

        // Calculate `rem` and combine to get final `v`
        // rem = (b & ((1 << (7 - len)) - 1))
        size_t rem = (b & ((1 << (7 - len)) - 1));

        // v = v + (rem << (8 * len))
        value += (rem << (8 * len));
      }
    }
    return value;
  }

  template <std::unsigned_integral I>
  void encodeJamCompactInteger(scale::ScaleEncoderStream &s, I integer) {
    if (integer < 128) {
      s << static_cast<uint8_t>(integer);
      return;
    }

    qtils::Bytes bytes;
    bytes.reserve(9);

    uint8_t prefix = 0;
    for (decltype(integer) i = integer; i; i = i >> 8) {
      if (i < 128 and (i >> 1 & prefix) == 0) {
        prefix |= i;
        break;
      }
      prefix = (prefix >> 1) | 0x80;
      bytes.emplace_back(static_cast<uint8_t>(i & 0xff));
    }

    s << prefix;
    for (auto byte : bytes) {
      s << byte;
    }
  }

  template <typename T>
  void decodeConfig(scale::ScaleDecoderStream &s, T &v, const auto &) {
    s >> v;
  }

  template <typename T>
  void encodeConfig(scale::ScaleEncoderStream &s, T &v, const auto &) {
    s << v;
  }

  void encodeConfig(scale::ScaleEncoderStream & /*s*/,
                    const jam::Empty & /*v*/,
                    const auto & /*config*/) {}

  void decodeConfig(scale::ScaleDecoderStream /*s*/,
                    jam::Empty & /*v*/,
                    const auto & /*config*/) {}

  template <typename T>
  void encodeConfig(scale::ScaleEncoderStream &s,
                    const std::vector<T> &v,
                    const auto &config) {
    encodeJamCompactInteger(s, v.size());
    for (const auto &item : v) {
      encodeConfig(s, item, config);
    }
  }

  template <typename T>
  void decodeConfig(scale::ScaleDecoderStream &s,
                    std::vector<T> &v,
                    const auto &config) {
    auto size = decodeJamCompactInteger(s);

    v.resize(size);
    for (size_t i = 0; i < size; ++i) {
      decodeConfig(s, v[i], config);
    }
  }

  template <typename T, size_t Size>
  void encodeConfig(scale::ScaleEncoderStream &s,
                    const std::array<T, Size> &v,
                    const auto &config) {
    for (const auto &item : v) {
      encodeConfig(s, item, config);
    }
  }

  template <typename T, size_t Size>
  void decodeConfig(scale::ScaleDecoderStream &s,
                    std::array<T, Size> &v,
                    const auto &config) {
    for (size_t i = 0; i < Size; ++i) {
      decodeConfig(s, v[i], config);
    }
  }

  template <typename T>
  void encodeConfig(scale::ScaleEncoderStream &s,
                    const std::optional<T> &v,
                    const auto &config) {
    if (not v.has_value()) {
      s << static_cast<uint8_t>(0);
    } else {
      s << static_cast<uint8_t>(1);
      encodeConfig(s, v.value(), config);
    }
  }

  template <typename T>
  void decodeConfig(scale::ScaleDecoderStream &s,
                    std::optional<T> &v,
                    const auto &config) {
    uint8_t i = 0;
    s >> i;
    if (i == 0) {
      v.reset();
    } else if (i == 1) {
      v.emplace();
      decodeConfig(s, v.value(), config);
    } else {
      raise(scale::DecodeError::WRONG_TYPE_INDEX);
    }
  }

  template <typename... Ts>
  void encodeConfig(scale::ScaleEncoderStream &s,
                    const boost::variant<Ts...> &v,
                    const auto &config) {
    s << static_cast<uint8_t>(v.which());
    boost::apply_visitor([&](const auto &v) { encodeConfig(s, v, config); }, v);
  }

  template <size_t I, typename T, typename... Ts>
  void decodeConfigBoostVariant(size_t i,
                                scale::ScaleDecoderStream &s,
                                auto &v,
                                const auto &config) {
    if (i == I) {
      decodeConfig(s, boost::get<T>(v), config);
    } else if constexpr (sizeof...(Ts) != 0) {
      decodeConfigBoostVariant<I + 1, Ts...>(i, s, v, config);
    }
  }

  template <typename... Ts>
  void decodeConfig(scale::ScaleDecoderStream &s,
                    boost::variant<Ts...> &v,
                    const auto &config) {
    uint8_t i = 0;
    s >> i;
    if (i < sizeof...(Ts)) {
      return decodeConfigBoostVariant<0, Ts...>(i, s, v, config);
    }
    raise(scale::DecodeError::WRONG_TYPE_INDEX);
  }

  template <typename... Ts>
  void encodeConfig(scale::ScaleEncoderStream &s,
                    const std::variant<Ts...> &v,
                    const auto &config) {
    s << static_cast<uint8_t>(v.index());
    std::visit([&](const auto &v) { encodeConfig(s, v, config); }, v);
  }

  template <size_t I, typename T, typename... Ts>
  void decodeConfigStdVariant(size_t i,
                              scale::ScaleDecoderStream &s,
                              auto &v,
                              const auto &config) {
    if (i == I) {
      v = T{};
      decodeConfig(s, std::get<T>(v), config);
    } else if constexpr (sizeof...(Ts) != 0) {
      decodeConfigStdVariant<I + 1, Ts...>(i, s, v, config);
    }
  }

  template <typename... Ts>
  void decodeConfig(scale::ScaleDecoderStream &s,
                    std::variant<Ts...> &v,
                    const auto &config) {
    uint8_t i = 0;
    s >> i;
    if (i < sizeof...(Ts)) {
      return decodeConfigStdVariant<0, Ts...>(i, s, v, config);
    }
    raise(scale::DecodeError::WRONG_TYPE_INDEX);
  }

  template <typename T, typename ConfigField>
  void encodeConfig(scale::ScaleEncoderStream &s,
                    const ConfigVec<T, ConfigField> &v,
                    const auto &config) {
    auto n = v.configSize(config);
    assert(v.v.size() == n);
    for (auto &item : v.v) {
      encodeConfig(s, item, config);
    }
  }

  template <typename T, typename ConfigField>
  void decodeConfig(scale::ScaleDecoderStream &s,
                    ConfigVec<T, ConfigField> &v,
                    const auto &config) {
    auto n = v.configSize(config);
    v.v.resize(n);
    for (size_t i = 0; i < n; ++i) {
      decodeConfig(s, v.v[i], config);
    }
  }

  template <typename T>
  auto encode(const T &v, const auto &config) {
    scale::ScaleEncoderStream s;
    encodeConfig(s, v, config);
    return s.to_vector();
  }

}  // namespace jam
