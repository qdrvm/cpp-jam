/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

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
  };
}  // namespace jam::snp
