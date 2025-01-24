/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <vector>

#include <test-vectors/config.hpp>
#include <scale/scale.hpp>

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
    friend scale::ScaleEncoderStream &operator<<(scale::ScaleEncoderStream &s,
                                                 const ConfigVec &v) {
      const auto &config = s.getConfig<test_vectors::Config>();
      auto n = v.configSize(config);
      assert(v.size() == n);
      for (auto &item : v) {
        s << item;
      }
      return s;
    }

    friend scale::ScaleDecoderStream &operator>>(scale::ScaleDecoderStream &s,
                                                 ConfigVec &v) {
      const auto &config = s.getConfig<test_vectors::Config>();
      auto n = v.configSize(config);
      v.resize(n);
      for (auto &item : v) {
        s >> item;
      }
      return s;
    }
  };
}  // namespace jam
