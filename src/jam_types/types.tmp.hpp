/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cinttypes>

#include <jam_types/common-types.hpp>

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

  using Block = test_vectors::Block;
  using BlockHeader = test_vectors::Header;

  // networking types

  using PeerId = qtils::Tagged<Stub, struct PeerId_>;  // STUB

  /// Direction, in which to retrieve ordered data
  enum class Direction : uint8_t {
    /// from child to parent
    ASCENDING = 0,
    /// from parent to canonical child
    DESCENDING = 1
  };

  /// Masks of bits, combination of which shows, which fields are to be
  /// presented in the BlockResponse
  enum class BlockAttribute : uint8_t {
    /// Include block header.
    HEADER = 1u << 0u,
    /// Include block body.
    BODY = 1u << 1u,
    _MASK = 0b11111,
  };
  inline constexpr auto operator|(BlockAttribute lhs, BlockAttribute rhs) {
    return static_cast<BlockAttribute>(static_cast<uint8_t>(lhs)
                                       | static_cast<uint8_t>(rhs));
  }
  inline constexpr auto operator&(BlockAttribute lhs, BlockAttribute rhs) {
    return static_cast<BlockAttribute>(static_cast<uint8_t>(lhs)
                                       & static_cast<uint8_t>(rhs));
  }

  /// Request for blocks to another peer
  struct BlocksRequest {
    /// bits, showing, which parts of BlockData to return
    BlockAttribute fields{};
    /// start from this block
    BlockIndex from{};
    /// sequence direction
    Direction direction{};
    /// maximum number of blocks to return; an implementation defined maximum is
    /// used when unspecified
    std::optional<uint32_t> max{};
    bool multiple_justifications = true;

    /// includes HEADER, BODY
    static constexpr BlockAttribute kBasicAttributes =
        BlockAttribute::HEADER | BlockAttribute::BODY;
  };

  struct BlockAnnounce {
    BlockAnnounce(const BlockAnnounce &) = delete;
  };

}  // namespace jam

SCALE_DEFINE_ENUM_VALUE_RANGE(jam,
                              Direction,
                              jam::Direction::ASCENDING,
                              jam::Direction::DESCENDING);
