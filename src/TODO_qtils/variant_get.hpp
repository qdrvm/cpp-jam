/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <variant>

namespace qtils {
  /**
   * Some people don't like pointer in
   *   if (auto *t = std::get_if<T>(&v))
   */
  template <typename T, typename V>
  struct VariantGet {
    VariantGet(V &variant) : variant_{variant} {}
    operator bool() const {
      return std::holds_alternative<T>(variant_);
    }
    auto &operator*() {
      return std::get<T>(variant_);
    }
    auto *operator->() {
      return &std::get<T>(variant_);
    }
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
    V &variant_;
  };

  template <typename T, typename... V>
  auto variantGet(const std::variant<V...> &variant) {
    return VariantGet<T, decltype(variant)>{variant};
  }

  template <typename T, typename... V>
  auto variantGet(std::variant<V...> &variant) {
    return VariantGet<T, decltype(variant)>{variant};
  }
}  // namespace qtils
