/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cinttypes>

#include <jam_types/common-types.hpp>
#include <scale/tie_hash.hpp>

namespace jam {
  // stub types. must be refactored in future

  struct Stub {};

  // crypto types

  using Hash64 = qtils::ByteArr<8>;
  using Hash128 = qtils::ByteArr<16>;
  using Hash256 = qtils::ByteArr<32>;
  using Hash512 = qtils::ByteArr<64>;

  // blockchain types

  using BlockHash = test_vectors::HeaderHash;

  using test_vectors::TimeSlot;

  struct BlockIndex {
    TimeSlot slot;
    BlockHash hash;
    auto operator<=>(const BlockIndex &other) const = default;
  };

}  // namespace jam
SCALE_TIE_HASH_STD(jam::BlockIndex);
namespace jam {

  using BlockInfo = BlockIndex;

  using BlockNumber = test_vectors::TimeSlot;

  using BlockId = std::variant<TimeSlot, BlockHash>;

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

template <>
struct fmt::formatter<jam::BlockInfo> {
  // Presentation format: 's' - short, 'l' - long.
  char presentation = 's';

  // Parses format specifications of the form ['s' | 'l'].
  constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) {
    // Parse the presentation format and store it in the formatter:
    auto it = ctx.begin(), end = ctx.end();
    if (it != end && (*it == 's' or *it == 'l')) {
      presentation = *it++;
    }

    // Check if reached the end of the range:
    if (it != end && *it != '}') {
      throw format_error("invalid format");
    }

    // Return an iterator past the end of the parsed range:
    return it;
  }

  // Formats the BlockInfo using the parsed format specification (presentation)
  // stored in this formatter.
  template <typename FormatContext>
  auto format(const jam::BlockInfo &block_info, FormatContext &ctx) const
      -> decltype(ctx.out()) {
    // ctx.out() is an output iterator to write to.

    if (presentation == 's') {
      return fmt::format_to(
          ctx.out(), "{:0x} @ {}", block_info.hash, block_info.slot);
    }

    return fmt::format_to(
        ctx.out(), "{:0xx} @ {}", block_info.hash, block_info.slot);
  }
};


template <typename T, typename U>
struct fmt::formatter<qtils::Tagged<T, U>> : formatter<T> {};
