/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jam/ed25519.hpp>
#include <jam/genesis_hash.hpp>
#include <jam/snp/connections/port.hpp>

namespace jam::snp {
  struct ConnectionsConfig {
    // https://github.com/zdave-parity/jam-np/blob/5d374b53578cdd93646e3ee19e2b19ea132317b8/simple.md?plain=1#L30-L35
    GenesisHash genesis;
    ed25519::KeyPair keypair;
    std::optional<Port> listen_port;
  };
}  // namespace jam::snp
