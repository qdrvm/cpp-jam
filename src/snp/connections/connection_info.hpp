/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "snp/connections/connection_id.hpp"
#include "snp/connections/key.hpp"

namespace jam::snp {
  struct ConnectionInfo {
    ConnectionId id;
    Key key;
    bool outbound;

    bool operator==(const ConnectionInfo &) const = default;
  };
}  // namespace jam::snp
