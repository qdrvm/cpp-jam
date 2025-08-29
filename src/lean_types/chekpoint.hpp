/**
* Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <qtils/byte_arr.hpp>
#include "types.tmp.hpp"

namespace lean {

  struct Checkpoint {
    qtils::ByteArr<32> root;
    Slot slot;
  };

}  // namespace lean
