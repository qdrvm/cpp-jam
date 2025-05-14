/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string_view>

#include <jam_types/types.tmp.hpp>
#include <qtils/byte_arr.hpp>

namespace jam::crypto {

  /**
   * Take a SHA-256 hash from string
   * @param input to be hashed
   * @return hashed bytes
   */
  Hash256 sha256(std::string_view input);

  /**
   * Take a SHA-256 hash from bytes
   * @param input to be hashed
   * @return hashed bytes
   */
  Hash256 sha256(qtils::ByteView input);

}  // namespace jam::crypto
