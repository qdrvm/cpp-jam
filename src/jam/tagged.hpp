/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <type_traits>
#include <utility>

#include <scale/scale.hpp>

namespace jam {

  template <typename T, typename = std::enable_if<std::is_scalar_v<T>>>
  struct Wrapper {
    template <typename... Args>
    Wrapper(Args &&...args) : value(std::forward<T>(args)...) {}

   protected:
    T value;
  };

  template <typename T,
            typename Tag,
            typename Base =
                std::conditional_t<std::is_scalar_v<T>, Wrapper<T>, T>>
  class Tagged : public Base {
   public:
    using type = T;
    using tag = Tag;

    Tagged() : Base() {}

    Tagged(T value) : Base(std::move(value)) {}

    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
    Tagged &operator=(T &&value) noexcept(
        not std::is_lvalue_reference_v<decltype(value)>) {
      if constexpr (std::is_scalar_v<T>) {
        this->Wrapper<T>::value = std::forward<T>(value);
      } else {
        static_cast<Base &>(*this) = std::forward<T>(value);
      }
      return *this;
    }

    template <typename Out>
    explicit operator Out() const {
      // NOLINTNEXTLINE(readability-else-after-return)
      if constexpr (std::is_scalar_v<T>) {
        return this->Wrapper<T>::value;
      } else {
        return *this;
      }
    }

    auto operator<=>(const Tagged<T, Tag> &other) const = default;
    bool operator==(const Tagged<T, Tag> &other) const = default;

   private:
    friend inline ::scale::ScaleEncoderStream &operator<<(
        ::scale::ScaleEncoderStream &s, const Tagged<T, Tag> &tagged) {
      if constexpr (std::is_scalar_v<T>) {
        return s << tagged.Wrapper<T>::value;
      } else {
        return s << static_cast<const T &>(tagged);
      }
    }

    friend inline ::scale::ScaleDecoderStream &operator>>(
        ::scale::ScaleDecoderStream &s, Tagged<T, Tag> &tagged) {
      if constexpr (std::is_scalar_v<T>) {
        s >> tagged.Wrapper<T>::value;
      } else {
        s >> static_cast<T &>(tagged);
      }
      return s;
    }
  };

  template <
      typename T,
      typename Tag,
      typename Base = std::conditional_t<std::is_scalar_v<T>, Wrapper<T>, T>>
  std::ostream &operator<<(std::ostream &os,
                           const Tagged<T, Tag, Base> &view) = delete;

  template <
      typename T,
      typename Tag,
      typename Base = std::conditional_t<std::is_scalar_v<T>, Wrapper<T>, T>>
  void encodeConfig(scale::ScaleEncoderStream &s,
                    const Tagged<T, Tag, Base> &tagged,
                    const auto &config) {
    if constexpr (std::is_scalar_v<T>) {
      encodeConfig(s, tagged.Wrapper<const T>::value, config);
    } else {
      encodeConfig(s, static_cast<const T &>(tagged), config);
    }
  }

  template <
      typename T,
      typename Tag,
      typename Base = std::conditional_t<std::is_scalar_v<T>, Wrapper<T>, T>>
  void decodeConfig(scale::ScaleDecoderStream &s,
                    Tagged<T, Tag, Base> &tagged,
                    const auto &config) {
    if constexpr (std::is_scalar_v<T>) {
      decodeConfig(s, tagged.Wrapper<T>::value, config);
    } else {
      decodeConfig(s, static_cast<T &>(tagged), config);
    }
  }

}  // namespace jam