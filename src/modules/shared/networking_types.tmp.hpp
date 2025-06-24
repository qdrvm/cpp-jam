/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "jam_types/block.hpp"
#include "jam_types/block_header.hpp"
#include "jam_types/block_request.hpp"
#include "jam_types/peer_id.hpp"
#include "utils/request_id.hpp"

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

  struct BlockAnnounceMessage {
    BlockAnnounce header;
    PeerId peer;
  };

  struct BlockRequestMessage {
    RequestCxt ctx;
    BlocksRequest request;
    PeerId peer;
  };

  struct BlockResponseMessage {
    RequestCxt ctx;
    outcome::result<Block> result;
    PeerId peer;
  };

}  // namespace jam::messages
