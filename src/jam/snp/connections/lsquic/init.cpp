/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <jam/snp/connections/lsquic/init.hpp>

#include <lsquic.h>
#include <jam/snp/connections/error.hpp>

namespace jam::snp::lsquic {
  outcome::result<void> init() {
    static auto ok = [] {
      return lsquic_global_init(LSQUIC_GLOBAL_CLIENT | LSQUIC_GLOBAL_SERVER)
          == 0;
    }();
    if (not ok) {
      return LsQuicError::lsquic_global_init;
    }
    return outcome::success();
  }
}  // namespace jam::snp::lsquic
