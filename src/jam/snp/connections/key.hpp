/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jam/ed25519.hpp>

namespace jam::snp {
  // https://github.com/zdave-parity/jam-np/blob/5d374b53578cdd93646e3ee19e2b19ea132317b8/simple.md?plain=1#L13-L14
  using Key = ed25519::Public;
}  // namespace jam::snp
