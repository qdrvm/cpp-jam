/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <vector>

namespace jam {
  template <typename T, typename ConfigField>
  struct ConfigVec {
    std::vector<T> v;

    static size_t configSize(const auto &config) {
      return config.get(ConfigField{});
    }

    bool operator==(const ConfigVec &) const = default;
  };
}  // namespace jam
