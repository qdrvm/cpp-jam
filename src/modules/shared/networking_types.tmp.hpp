/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "jam_types/types.tmp.hpp"
#include "utils/request_id.hpp"

namespace jam::messages {
  template <typename Notification>
  struct NotificationReceived {
    PeerId from_peer;
    Notification notification;
  };

  template <typename Notification>
  struct BroadcastNotification {
    Notification notification;
  };

  template <typename Request>
  struct RequestReceived {
    RequestCxt ctx;
    PeerId from_peer;
    Request request;
  };

  template <typename Request>
  struct SendRequest {
    RequestCxt ctx;
    PeerId to_peer;
    Request request;
  };

  template <typename Response>
  struct ResponseReceived {
    RequestCxt for_ctx;
    outcome::result<Response> response_result;
  };

  template <typename Response>
  struct SendResponse {
    RequestCxt for_ctx;
    outcome::result<Response> response_result;
  };

  struct PeerConnectedMessage {
    PeerId peer;
    // address?
    // initial view?
  };

  struct PeerDisconnectedMessage {
    PeerId peer;
    // reason?
  };

  using BlockAnnouncementHandshakeReceived =
      NotificationReceived<BlockAnnouncementHandshake>;
  using BlockAnnouncementReceived = NotificationReceived<BlockAnnouncement>;

  using SendBlockRequest = SendRequest<BlockRequest>;
  using BlockResponseReceived = ResponseReceived<BlockResponse>;
}  // namespace jam::messages
