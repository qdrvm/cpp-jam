/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <atomic>
#include <memory>

#include "snp/connections/connection_id.hpp"

namespace jam::snp {
  class ConnectionIdCounter {
   public:
    ConnectionId make() {
      return connection_id_->fetch_add(1);
    }

   private:
    using Atomic = std::atomic<ConnectionId>;
    std::shared_ptr<Atomic> connection_id_ = std::make_shared<Atomic>();
  };
}  // namespace jam::snp
