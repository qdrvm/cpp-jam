/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "storage/rocksdb/rocksdb_spaces.hpp"

#include <algorithm>
#include <array>

#include <boost/assert.hpp>
#include <rocksdb/db.h>

namespace jam::storage {

  static constexpr std::array kNames{
      "lookup_key",
      "trie_node",
      "justification",
  };
  static_assert(kNames.size() == SpacesCount - 1);

  std::string spaceName(Space space) {
    static const std::vector<std::string> names = []() {
      std::vector<std::string> names;
      names.push_back(rocksdb::kDefaultColumnFamilyName);
      names.insert(names.end(), kNames.begin(), kNames.end());
      return names;
    }();
    BOOST_ASSERT(space < Space::Total);
    return names.at(static_cast<size_t>(space));
  }

  std::optional<Space> spaceFromString(std::string_view string) {
    std::optional<Space> space;
    const auto it = std::ranges::find(kNames, string);
    if (it != std::end(kNames)) {
      space.emplace(static_cast<Space>(std::distance(std::begin(kNames), it)));
    }
    return space;
  }

}  // namespace jam::storage
