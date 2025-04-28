/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cinttypes>

#include <jam_types/common-types.hpp>

#include "snp/peer_id.hpp"

namespace jam {

  // stub types. must be refactored in future

  struct Stub {};

  // blockchain types

  using BlockNumber = uint32_t;
  using BlockHash = test_vectors::HeaderHash;

  struct BlockIndex {
    BlockNumber number;
    BlockHash hash;
  };

  // Different from `BlockIndex { BlockNumber BlockHash }`.
  // Encodes hash before slot, so can't sort keys by number.
  struct BlockHashSlot {
    BlockHash hash;
    BlockNumber slot;
  };

  using Block = test_vectors::Block;
  using BlockHeader = test_vectors::Header;

  // networking types

  /// Direction, in which to retrieve ordered data
  enum class Direction : uint8_t {
    /// from child to parent
    ASCENDING = 0,
    /// from parent to canonical child
    DESCENDING = 1
  };


  // https://github.com/zdave-parity/jam-np/blob/main/simple.md#ce-128-block-request
  /// Request for blocks to another peer
  struct BlockRequest {
    /// start from this block
    BlockHash from{};
    /// sequence direction
    Direction direction{};
    /// maximum number of blocks to return; an implementation defined maximum is
    /// used when unspecified
    uint32_t max{};
  };

  // https://github.com/zdave-parity/jam-np/blob/main/simple.md#ce-128-block-request
  struct BlockResponse {
    // TODO: custom encode/decode without length prefix
    std::vector<Block> blocks;
  };

  // https://github.com/zdave-parity/jam-np/blob/main/simple.md#up-0-block-announcement
  struct BlockAnnouncementHandshake {
    BlockHashSlot finalized;
    std::vector<BlockHashSlot> leafs;
  };

  // https://github.com/zdave-parity/jam-np/blob/main/simple.md#up-0-block-announcement
  struct BlockAnnouncement {
    BlockHeader block_header;
    BlockHashSlot finalized;
  };

}  // namespace jam

SCALE_DEFINE_ENUM_VALUE_RANGE(jam,
                              Direction,
                              jam::Direction::ASCENDING,
                              jam::Direction::DESCENDING);

template <>
struct fmt::formatter<jam::Stub> {
  constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) {
    auto it = ctx.begin(), end = ctx.end();
    if (it != end && *it != '}') {
      throw format_error("invalid format");
    }
    return it;
  }

  template <typename FormatContext>
  auto format(const jam::Stub &, FormatContext &ctx) const
      -> decltype(ctx.out()) {
    return fmt::format_to(ctx.out(), "stub");
  }
};

template <typename T, typename U>
struct fmt::formatter<qtils::Tagged<T, U>> : formatter<T> {};
