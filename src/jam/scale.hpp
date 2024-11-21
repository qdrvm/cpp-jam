/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ranges>
#include <scale/scale.hpp>
#include <span>

namespace jam {
  template <typename T>
  class NoLenWrapper : private std::span<T> {
    using Base = std::span<T>;

   public:
    NoLenWrapper(std::span<T> &&range)
        : Base(std::forward<std::span<T>>(range)) {}

    inline friend auto &operator<<(
        scale::ScaleEncoderStream &s, const NoLenWrapper &v) {
      for (const auto &item : v) {
        s << item;
      }
      return s;
    }
  };

  template <typename T>
  concept RangeOf = std::ranges::range<T>;

  template <RangeOf R>
  auto WithoutLength(const R &range) {
    using T = const std::decay_t<std::ranges::range_value_t<R>>;
    return NoLenWrapper<T>(std::span<T>{range});
  }

}  // namespace jam
