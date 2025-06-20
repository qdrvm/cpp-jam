/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "storage/rocksdb/rocksdb_spaces.hpp"

#include <algorithm>
#include <array>
#include <span>

#include <boost/assert.hpp>
#include <boost/static_assert.hpp>
#include <rocksdb/db.h>

namespace jam::storage {

  static constexpr std::string_view kNamesArr[] = {
      ""
      // Add here names of non-default space
  };
  constexpr std::span<const std::string_view> kNames = kNamesArr;

  // static_assert(kNames.size() == (SpacesCount - 1));

  std::string_view spaceName(Space space) {
    if (space != Space::Default) {
      BOOST_ASSERT(space < Space::Total);
      return kNames[static_cast<size_t>(space) - 1];
    }
    return rocksdb::kDefaultColumnFamilyName;
  }

  std::optional<Space> spaceFromString(std::string_view string) {
    std::optional<Space> space;
    const auto it = std::find(std::begin(kNames), std::end(kNames), string);
    if (it != std::end(kNames)) {
      space.emplace(static_cast<Space>(std::distance(std::begin(kNames), it)));
    }
    return space;
  }

}  // namespace jam::storage
