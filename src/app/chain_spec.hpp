/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string>
#include <vector>

#include <qtils/byte_vec.hpp>

#include "jam_types/types.tmp.hpp"

namespace jam {

  using NodeAddress = Stub;

}

namespace jam::app {

  class ChainSpec {
   public:
    virtual ~ChainSpec() = default;

    virtual const std::string &id() const = 0;

    virtual const std::vector<NodeAddress> &bootNodes() const = 0;

    virtual const qtils::ByteVec &genesisHeader() const = 0;

    virtual const std::map<qtils::ByteVec, qtils::ByteVec> &genesisState()
        const = 0;
  };

}  // namespace jam::app
