/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <utility>

namespace qtils {
  template <typename T>
  size_t stdHashOf(const T &v) {
    return std::hash<T>()(v);
  }
}  // namespace qtils
