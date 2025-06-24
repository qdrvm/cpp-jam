/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <fmt/core.h>
#include <qtils/empty.hpp>

template <>
struct fmt::formatter<qtils::Empty> {
  constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) {
    return ctx.begin();
  }
  auto format(const qtils::Empty &, format_context &ctx) const
      -> decltype(ctx.out()) {
    return ctx.out();
  }
};
