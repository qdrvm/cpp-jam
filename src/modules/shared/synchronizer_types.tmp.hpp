/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "jam_types/types.tmp.hpp"

namespace jam::messages {

  struct BlockDiscoveredMessage {
    BlockIndex index;
    PeerId peer;
  };

}  // namespace jam::messages
