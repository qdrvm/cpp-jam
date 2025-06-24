/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "jam_types/block_index.hpp"
#include "jam_types/peer_id.hpp"

namespace jam::messages {

  struct BlockDiscoveredMessage {
    BlockIndex index;
    PeerId peer;
  };

}