/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "snp/connections/connection_ptr.hpp"
#include "snp/connections/key.hpp"

namespace jam::snp {
  class ConnectionsController {
   public:
    virtual ~ConnectionsController() = default;

    /**
     * There is now some connection with peer.
     */
    virtual void onOpen(Key key) {}

    /**
     * There are no more connections with peer.
     */
    virtual void onClose(Key key) {}

    /**
     * Called when first connection was opened.
     * Called when first connection was replaced by second connection.
     * Can be used to open notification streams.
     */
    virtual void onConnectionChange(ConnectionPtr connection) {}
  };
}  // namespace jam::snp
