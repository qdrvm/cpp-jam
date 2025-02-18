/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <optional>

namespace qtils {
  /**
   * `std::optional` wrapper.
   * Becomes `std::nullopt` when moved from.
   * `std::nullopt` throws on access to value.
   * Requires to write `.has_value()`.
   * Can be move assigned for move only types.
   */
  template <typename T>
  class Optional {
   public:
    Optional() : value_{std::nullopt} {}

    Optional(const std::nullopt_t &) : value_{std::nullopt} {}

    Optional(const T &value) : value_{value} {}
    void operator=(const T &value) {
      value_.emplace(value);
    }

    Optional(T &&value) : value_{std::move(value)} {}
    void operator=(T &&value) {
      value_.emplace(std::move(value));
    }

    Optional(const Optional<T> &other) : value_{other.value_} {}
    void operator=(const Optional<T> &other) {
      value_ = other.value_;
    }

    Optional(Optional<T> &&other)
        : value_{std::exchange(other.value_, std::nullopt)} {}
    void operator=(Optional<T> &&other) {
      if (other.value_.has_value()) {
        value_.emplace(std::exchange(other.value_, std::nullopt).value());
      } else {
        value_.reset();
      }
    }

    bool has_value() const {
      return value_.has_value();
    }

    T &operator*() {
      return value_.value();
    }
    const T &operator*() const {
      return value_.value();
    }
    T *operator->() {
      return &value_.value();
    }
    const T *operator->() const {
      return &value_.value();
    }

    Optional<T> take() {
      return std::exchange(*this, Optional<T>{});
    }

    bool operator==(const T &value) const {
      return value_ == value;
    }

   private:
    std::optional<T> value_;
  };
}  // namespace qtils
