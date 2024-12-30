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
#include <src/jam/empty.hpp>

#include <boost/variant.hpp>
#include <qtils/bytes.hpp>

#include <test-vectors/config-types.hpp>
#include <test-vectors/config.hpp>

namespace jam {

  template <typename... Ts>
  scale::ScaleEncoderStream &operator<<(scale::ScaleEncoderStream &s,
                                        const std::variant<Ts...> &v) {
    s << static_cast<uint8_t>(v.index());
    std::visit([&](const auto &v) { s << v; }, v);
    return s;
  }

  template <size_t I, typename T, typename... Ts>
  void decodeConfigStdVariant(size_t i, scale::ScaleDecoderStream &s, auto &v) {
    if (i == I) {
      v = T{};
      s >> std::get<T>(v);
    } else if constexpr (sizeof...(Ts) != 0) {
      decodeConfigStdVariant<I + 1, Ts...>(i, s, v);
    }
  }

  template <typename... Ts>
  scale::ScaleDecoderStream &operator>>(scale::ScaleDecoderStream &s,
                                        std::variant<Ts...> &v) {
    uint8_t i = 0;
    s >> i;
    if (i < sizeof...(Ts)) {
      decodeConfigStdVariant<0, Ts...>(i, s, v);
      return s;
    }
    raise(scale::DecodeError::WRONG_TYPE_INDEX);
  }

  template <typename T, typename ConfigField>
  scale::ScaleEncoderStream &operator<<(scale::ScaleEncoderStream &s,
                                        const ConfigVec<T, ConfigField> &v) {
    auto &config = s.getConfig<jam::test_vectors::Config>();
    auto n = v.configSize(config);
    assert(v.v.size() == n);
    for (auto &item : v.v) {
      s << item;
    }
    return s;
  }

  template <typename T, typename ConfigField>
  scale::ScaleDecoderStream &operator>>(scale::ScaleDecoderStream &s,
                                        ConfigVec<T, ConfigField> &v) {
    auto &config = s.getConfig<jam::test_vectors::Config>();
    auto n = v.configSize(config);
    v.v.resize(n);
    for (auto &item : v.v) {
      s >> item;
    }
    return s;
  }

  template <typename T>
  outcome::result<scale::ByteArray> encode(const T &v, const auto &config) {
    scale::ScaleEncoderStream s(config);
    OUTCOME_TRY(encode(s, v));
    return outcome::success(s.to_vector());
  }

  template <typename T>
  outcome::result<T> decode(qtils::BytesIn bytes, const auto &config) {
    scale::ScaleDecoderStream s(bytes, config);
    T t;
    OUTCOME_TRY(scale::decode<T>(s, t));
    return outcome::success(std::move(t));
  }

}  // namespace jam
