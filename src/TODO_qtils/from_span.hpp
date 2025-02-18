/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstring>
#include <optional>
#include <stdexcept>

#include <TODO_qtils/optional.hpp>
#include <qtils/bytes.hpp>

namespace qtils {
  inline bool fromSpan(BytesOut out, BytesIn span) {
    if (span.size() != out.size()) {
      return false;
    }
    memcpy(out.data(), span.data(), out.size());
    return true;
  }

  template <typename T>
  Optional<T> fromSpan(BytesIn span) {
    T out;
    if (not fromSpan(out, span)) {
      return std::nullopt;
    }
    return out;
  }
}  // namespace qtils
