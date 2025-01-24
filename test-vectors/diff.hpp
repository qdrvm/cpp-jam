/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <array>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

#include <qtils/hex.hpp>

#include <jam/empty.hpp>
#include <jam/tagged.hpp>
#include <test-vectors/config-types.hpp>

/**
 * Print colorful diff for objects.
 * Used for debugging tests.
 */

struct Indent {
  int indent = 0;
  Indent operator~() const {
    return Indent{indent + 1};
  }
};
template <>
struct fmt::formatter<Indent> {
  static constexpr auto parse(format_parse_context &ctx) {
    return ctx.begin();
  }
  auto format(const Indent &v, format_context &ctx) const {
    auto out = ctx.out();
    for (int i = 0; i < v.indent; ++i) {
      out = fmt::detail::write(out, "  ");
    }
    return out;
  }
};

#define DIFF_F(...) \
  void diff(Indent indent, const __VA_ARGS__ &v1, const __VA_ARGS__ &v2)

#define DIFF_M(m) diff_m(indent, v1.m, v2.m, #m)

enum class color { red = 31, green = 32 };
template <typename... T>
void _diff(color color, fmt::format_string<T...> f, T &&...a) {
  fmt::print("\x1b[0;{}m", (int)color);
  fmt::print(f, static_cast<T &&>(a)...);
  fmt::println("\x1b[0;0m");
}
template <typename... T>
void diff1(fmt::format_string<T...> f, T &&...a) {
  _diff(color::red, f, static_cast<T &&>(a)...);
}
template <typename... T>
void diff2(fmt::format_string<T...> f, T &&...a) {
  _diff(color::green, f, static_cast<T &&>(a)...);
}

template <typename T>
void diff_m(Indent indent, const T &v1, const T &v2, std::string_view name) {
  if (v1 == v2) {
    return;
  }
  fmt::println("{}{}", indent, name);
  diff(~indent, v1, v2);
}

DIFF_F(uint32_t) {
  fmt::println("{}{} != {}", indent, v1, v2);
}

DIFF_F(jam::Empty) {}

//template <typename T, typename Tag>
//DIFF_F(jam::Tagged<T, Tag>) {
//  diff(indent, (const T &)v1, (const T &)v2);
//}

// template <typename T, typename ConfigField>
// DIFF_F(jam::ConfigVec<T, ConfigField>) {
//   diff(indent, v1, v2);
// }

template <typename T>
DIFF_F(std::optional<T>) {
  if (v1 == v2) {
    return;
  }
  std::string_view o[] = {"None", "Some"};
  auto i1 = v1 ? 1 : 0, i2 = v2 ? 1 : 0;
  if (i1 != i2) {
    fmt::println("{}{} != {}", indent, o[i1], o[i2]);
    return;
  }
  fmt::println("{}{}", indent, o[i1]);
  if (v1) {
    diff(~indent, *v1, *v2);
  }
}

template <typename T>
  requires(
      requires(T v) { std::span{v}; }
      and not requires(T v) { qtils::BytesIn{v}; })
void diff(Indent indent, const T &_v1, const T &_v2) {
  if (_v1 == _v2) {
    return;
  }
  std::span v1{_v1}, v2{_v2};
  if (v1.size() != v2.size()) {
    fmt::println("{}{} != {}", indent, v1.size(), v2.size());
    return;
  }
  fmt::println("{}{}", indent, v1.size());
  for (size_t i = 0; i < v1.size(); ++i) {
    if (v1[i] == v2[i]) {
      continue;
    }
    fmt::println("{}[{}]", indent, i);
    diff(~indent, v1[i], v2[i]);
  }
}

DIFF_F(qtils::BytesIn) {
  if (v1.size() == v2.size() && memcmp(v1.data(), v2.data(), v1.size()) == 0) {
    return;
  }
  diff1("{}- {:x}", indent, v1);
  diff2("{}+ {:x}", indent, v2);
}

template <typename E>
  requires(std::is_enum_v<E>)
void diff_e(Indent indent, const E &v1, const E &v2, const auto &names) {
  if (v1 == v2) {
    return;
  }

  auto get_name = [&](auto v) {
    using IntType = std::decay_t<decltype(names)>::key_type;
    if (auto it = names.find((IntType)v); it != names.end()) {
      return std::string(it->second);
    }
    return fmt::format("<wrong_value:{}>", (IntType)v);
  };

  fmt::println("{}{} != {}", indent, get_name(v1), get_name(v2));
}

template <typename... Ts>
void diff_v(Indent indent,
            const std::variant<Ts...> &v1,
            const std::variant<Ts...> &v2,
            std::span<const std::string_view> tags) {
  if (v1 == v2) {
    return;
  }
  if (v1.index() != v2.index()) {
    fmt::println("{}{} != {}", indent, tags[v1.index()], tags[v2.index()]);
    return;
  }

  fmt::println("{}{}", indent, tags[v1.index()]);
  std::visit(
      [&](auto &&v) {
        diff(~indent, v, std::get<std::decay_t<decltype(v)>>(v2));
      },
      v1);
}
