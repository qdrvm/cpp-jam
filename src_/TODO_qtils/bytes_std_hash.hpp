/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <qtils/bytes.hpp>
#include <qtils/bytestr.hpp>

namespace qtils {
  struct BytesStdHash {
    std::size_t operator()(qtils::BytesIn bytes) const {
      return std::hash<std::string_view>{}(qtils::byte2str(bytes));
    }
  };
}  // namespace qtils
