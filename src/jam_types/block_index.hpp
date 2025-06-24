/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <scale/tie_hash.hpp>

#include "jam_types/types.hpp"

namespace jam {

  struct BlockIndex {
    TimeSlot slot;
    HeaderHash hash;
    auto operator<=>(const BlockIndex &other) const = default;
  };

  TEMPORARY_TYPE_ALIAS(BlockInfo, BlockIndex);
  TEMPORARY_TYPE_ALIAS(BlockNumber, TimeSlot);
  TEMPORARY_TYPE_ALIAS(BlockId, std::variant<TimeSlot, HeaderHash>);

}  // namespace jam

SCALE_TIE_HASH_STD(jam::BlockIndex);

template <>
struct fmt::formatter<jam::BlockIndex> {
  // Presentation format
  bool long_form = false;

  // Parses format specifications of the form ['s' | 'l'].
  constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) {
    auto it = ctx.begin(), end = ctx.end();
    if (it != end) {
      if (*it == 'l' or *it == 's') {
        long_form = *it == 'l';
        ++it;
      }
    }
    if (it != end && *it != '}') {
      throw format_error("invalid format");
    }
    return it;
  }

  // Formats the BlockIndex using the parsed format specification (presentation)
  // stored in this formatter.
  template <typename FormatContext>
  auto format(const jam::BlockIndex &block_index, FormatContext &ctx) const
      -> decltype(ctx.out()) {
    [[unlikely]] if (long_form) {
      return fmt::format_to(
          ctx.out(), "{:0xx} @ {}", block_index.hash, block_index.slot);
    }
    return fmt::format_to(
        ctx.out(), "{:0x} @ {}", block_index.hash, block_index.slot);
  }
};
