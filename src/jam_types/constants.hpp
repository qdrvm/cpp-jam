/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include <cstdint>

namespace jam {

  static constexpr uint64_t SLOT_DURATION_MS = 4000;  // 4 seconds
  static constexpr uint64_t INTERVALS_PER_SLOT = 4;   // 4 intervals	by 1 second

  // State list lengths

  static constexpr uint64_t HISTORICAL_ROOTS_LIMIT =  //
      1 << 18;  // 262'144 historical roots,	12.1 days
  static constexpr uint64_t VALIDATOR_REGISTRY_LIMIT =  //
      1 << 12;                                          // 4'096 validators

}  // namespace jam
