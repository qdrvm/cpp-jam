/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string>
#include <vector>

#include <qtils/byte_vec.hpp>

#include "jam_types/types.hpp"
#include "jam_types/peer_address.hpp"

namespace jam::app {

  class ChainSpec {
   public:
    virtual ~ChainSpec() = default;

    [[nodiscard]] virtual const std::string &id() const = 0;

    [[nodiscard]] virtual const std::vector<PeerAddress> &bootNodes() const = 0;

    [[nodiscard]] virtual const qtils::ByteVec &genesisHeader() const = 0;

    [[nodiscard]] virtual const std::map<qtils::ByteVec, qtils::ByteVec> &
    genesisState() const = 0;
  };

}  // namespace jam::app
