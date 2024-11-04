/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <scale/scale.hpp>
#include <test-vectors/config-types.hpp>

namespace scale {
  template <typename T>
  void decodeConfig(ScaleDecoderStream &s, T &v, const auto &config) {
    s >> v;
  }

  template <typename T>
  void decodeConfig(
      ScaleDecoderStream &s, std::vector<T> &v, const auto &config) {
    auto n = s.decodeLength();
    v.resize(0);
    v.reserve(n);
    for (size_t i = 0; i < n; ++i) {
      T item;
      decodeConfig(s, item, config);
      v.emplace_back(std::move(item));
    }
  }

  template <typename T>
  void decodeConfig(
      ScaleDecoderStream &s, std::optional<T> &v, const auto &config) {
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
  void decodeConfigVariant(
      size_t i, ScaleDecoderStream &s, auto &v, const auto &config) {
    if (i == I) {
      T item;
      decodeConfig(s, item, config);
      v = std::move(item);
    } else if constexpr (sizeof...(Ts) != 0) {
      decodeConfigVariant<I + 1, Ts...>(i, s, v, config);
    }
  }

  template <typename... Ts>
  void decodeConfig(
      ScaleDecoderStream &s, boost::variant<Ts...> &v, const auto &config) {
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
    auto n = config.get(ConfigField{});
    v.v.resize(0);
    v.v.reserve(n);
    for (size_t i = 0; i < n; ++i) {
      T item;
      decodeConfig(s, item, config);
      v.v.emplace_back(std::move(item));
    }
  }

  template <typename T, typename ConfigField>
  scale::ScaleEncoderStream &operator<<(
      scale::ScaleEncoderStream &s, const ConfigVec<T, ConfigField> &v) {
    for (auto &item : v.v) {
      s << item;
    }
    return s;
  }
}  // namespace jam
