/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <vector>

namespace jam {
  template <typename T, typename ConfigField>
  class ConfigVec : public std::vector<T> {
   public:
    ConfigVec() = default; // To make non-aggregate

    static size_t configSize(const auto &config) {
      return config.get(ConfigField{});
    }

   private:
    // Next methods are private to consider scale::StaticCollection concept
    using std::vector<T>::vector;
    using std::vector<T>::insert;
    using std::vector<T>::emplace;
  };
}  // namespace jam
