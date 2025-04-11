/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <concepts>

#include <morum/common.hpp>
#include <morum/tree_node.hpp>

namespace morum {

  template <typename T>
  concept StateDictionary = std::ranges::random_access_range<T>
                         && requires(std::ranges::range_value_t<T> value) {
                              {
                                std::get<0>(value)
                              } -> std::convertible_to<Hash32>;
                              {
                                std::get<1>(value)
                              } -> std::convertible_to<qtils::ByteSpan>;
                            };

  template <StateDictionary State>
  Hash32 calculate_root_in_memory(State &&state, size_t depth = 0) {
    if (std::ranges::empty(state)) {
      return ZeroHash32;
    }
    if (std::ranges::begin(state) + 1 == std::ranges::end(state)) {
      auto kv = std::ranges::begin(state);
      auto &&[k, v] = *kv;
      ByteArray<31> partial_key;
      std::copy_n(k.begin(), 31, partial_key.begin());
      return blake2b_256(serialize_leaf(partial_key, v));
    }
    auto right_subtree_it = std::ranges::lower_bound(
        state, std::ranges::range_value_t<State>{}, [depth](auto value, auto) {
          // bit #depth (starting from lsb) is 0
          return (std::get<0>(value)[depth / 8] & (0x01 << (depth % 8))) == 0;
        });

    auto left_root = calculate_root_in_memory(
        std::ranges::subrange(std::ranges::begin(state), right_subtree_it),
        depth + 1);
    auto right_root = calculate_root_in_memory(
        std::ranges::subrange(right_subtree_it, std::ranges::end(state)),
        depth + 1);

    return blake2b_256(serialize_branch(left_root, right_root));
  }

}  // namespace morum