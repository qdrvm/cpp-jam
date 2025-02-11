/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jam/snp/connections/connection_id.hpp>
#include <jam/snp/connections/key.hpp>

namespace jam::snp {
  struct ConnectionInfo {
    ConnectionId id;
    Key key;

    bool operator==(const ConnectionInfo &) const = default;
  };
}  // namespace jam::snp
