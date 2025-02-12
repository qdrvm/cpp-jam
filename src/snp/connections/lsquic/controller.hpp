/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "snp/connections/connection_info.hpp"
#include "snp/connections/connection_ptr.hpp"
#include "snp/connections/protocol_id.hpp"
#include "snp/connections/stream_ptr.hpp"

namespace jam::snp::lsquic {
  class EngineController {
   public:
    virtual ~EngineController() = default;

    /**
     * Connection was accepted.
     */
    virtual void onConnectionAccept(ConnectionPtr connection) {}

    /**
     * Connection was closed.
     */
    virtual void onConnectionClose(ConnectionInfo connection_info) {}

    /**
     * Stream was accepted.
     */
    virtual void onStreamAccept(ConnectionPtr connection,
                                ProtocolId protocol_id,
                                StreamPtr stream) {}
  };
}  // namespace jam::snp::lsquic
