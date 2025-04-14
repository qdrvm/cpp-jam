/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "jam_types/types.tmp.hpp"

namespace jam::messages {

  struct PeerConnectedMessage {
    PeerId peer;
    // address?
    // initial view?
  };

  struct PeerDisconnectedMessage {
    PeerId peer;
    // reason?
  };

  struct BlockAnnounce {
    BlockHeader header;
    PeerId peer;
  };

  struct BlockRequestMessage {
    BlocksRequest request;
    PeerId peer;
  };

  struct BlockResponseMessage {
    outcome::result<Block> result;
    PeerId peer;
  };

}