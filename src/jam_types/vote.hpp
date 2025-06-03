/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jam_types/chekpoint.hpp>

namespace jam {

  struct Vote {
    uint64_t validator_id;
    uint64_t slot;
    Checkpoint head;
    Checkpoint target;
    Checkpoint source;
  };

}  // namespace jam
