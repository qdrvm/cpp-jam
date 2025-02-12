/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "snp/connections/prefer_key.hpp"

namespace jam::snp {
  bool prefer_key(const Key &a, const Key &b) {
    return ((a[31] > 127) != (b[31] > 127)) != (a < b);
  }
}  // namespace jam::snp
