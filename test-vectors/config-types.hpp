/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <limits>
#include <vector>

#include <jam_types/config.hpp>
#include <scale/jam_scale.hpp>

namespace jam {
  template <typename T, typename ConfigField>
  class ConfigVec : public std::vector<T> {
   public:
    ConfigVec() = default;  // To make non-aggregate

    static size_t configSize(const auto &config) {
      return config.get(ConfigField{});
    }

   private:
    // Next methods are private to consider scale::StaticCollection concept
    using std::vector<T>::vector;
    using std::vector<T>::insert;
    using std::vector<T>::emplace;

   public:
    friend void encode(const ConfigVec &v, scale::ScaleEncoder auto &encoder) {
      const auto &config = encoder.template getConfig<test_vectors::Config>();
      auto n = v.configSize(config);
      if (n == std::numeric_limits<decltype(n)>::max()) {
        return encode(static_cast<const std::vector<T> &>(v), encoder);
      }
      assert(v.size() == n);
      for (auto &item : v) {
        encode(item, encoder);
      }
    }

    friend void decode(ConfigVec &v, scale::ScaleDecoder auto &decoder) {
      const auto &config = decoder.template getConfig<test_vectors::Config>();
      auto n = v.configSize(config);
      if (n == std::numeric_limits<decltype(n)>::max()) {
        return decode(static_cast<std::vector<T> &>(v), decoder);
      }
      v.resize(n);
      for (auto &item : v) {
        decode(item, decoder);
      }
    }
  };
}  // namespace jam
