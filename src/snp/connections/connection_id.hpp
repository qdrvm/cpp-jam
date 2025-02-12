/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>

namespace jam::snp {
  /**
   * Is not QUIC connection id.
   * Used to distinguish connections with same peer key.
   */
  using ConnectionId = uint64_t;
}  // namespace jam::snp
