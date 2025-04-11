/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "jam_types/types.tmp.hpp"

namespace jam::messages {

  struct BlockAnnounce {
  };

  struct BlockRequestMessage {
    BlockAnnounce header;
    PeerId peer;
  };

  struct BlockResponseMessage {
    outcome::result<Block> result;
    PeerId peer;
  };

}