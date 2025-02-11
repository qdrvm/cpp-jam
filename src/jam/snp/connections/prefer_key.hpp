/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jam/snp/connections/key.hpp>

namespace jam::snp {
  // https://github.com/zdave-parity/jam-np/blob/5d374b53578cdd93646e3ee19e2b19ea132317b8/simple.md?plain=1#L52-L62
  /**
   * Is first key preferred over second.
   */
  bool prefer_key(const Key &a, const Key &b);
}  // namespace jam::snp
