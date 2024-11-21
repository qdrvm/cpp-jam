/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include <algorithm>

namespace qtils::cxx23::ranges {
  auto contains(auto &&r, const auto &v) {
    auto end = std::ranges::end(r);
    return std::find(std::ranges::begin(r), end, v) != end;
  }

  auto contains_if(auto &&r, const auto &f) {
    auto end = std::ranges::end(r);
    return std::find_if(std::ranges::begin(r), end, f) != end;
  }
}  // namespace qtils::cxx23::ranges
