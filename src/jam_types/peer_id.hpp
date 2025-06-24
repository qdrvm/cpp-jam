/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

// #include <log/formatters/empty.hpp>
// #include <log/formatters/tagged.hpp>
// #include <qtils/empty.hpp>
#include <qtils/byte_arr.hpp>
// #include <qtils/tagged.hpp>
// #include <qtils/outcome.hpp>
// #include <qtils/enum_error_code.hpp>

namespace jam {

  class PeerId : qtils::ByteArr<32> {
   public:
    enum class Error {
      Empty,
      TooShort,
      TooLong,
      Malformed,
    };

    PeerId() = default;

    static outcome::result<PeerId> fromString(std::string_view base32);

    friend auto to_string(const PeerId &peer_id) -> std::string {
      return peer_id.str();
    }

    auto operator<=>(const PeerId &) const = default;

   private:
    [[nodiscard]] std::string str() const;
  };
}  // namespace jam

OUTCOME_HPP_DECLARE_ERROR(jam, PeerId::Error);

template <>
struct fmt::formatter<jam::PeerId> {
  bool long_form = false;

  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx) {
    constexpr bool is_compile =
        std::is_same_v<ParseContext, detail::compile_parse_context<char>>;

    auto it = ctx.begin();
    auto end = ctx.end();

    if (it != end) {
      if (*it == 'l') {
        long_form = true;
        ++it;
      } else if (*it == 's') {
        long_form = false;
        ++it;
      }
    }

    if (it == end or *it == '}') {
      return it;
    }

    if constexpr (is_compile) {
      report_error("invalid format specifier: expected [s|l]");
    } else {
      throw format_error("invalid format specifier for PeerId: expected [s|l]");
    }
  }

  auto format(const jam::PeerId &peer_id, format_context &ctx) const
      -> decltype(ctx.out()) {
    auto &&str = to_string(peer_id);
    std::span span(str);

    [[likely]] if (not long_form) { span = span.last(6); }

    auto out = detail::write(ctx.out(), "…");
    for (auto x : span) {
      *out = x;
      ++out;
    }
    return out;
  }
};
