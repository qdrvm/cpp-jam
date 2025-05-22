/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "blockchain/impl/justification_storage_policy.hpp"

#include <memory>

#include "blockchain/block_tree.hpp"

namespace jam::blockchain {

  outcome::result<bool> JustificationStoragePolicyImpl::shouldStoreFor(
      const BlockHeader &block_header,
      BlockNumber last_finalized_number) const {
    if (block_header.slot == 0) {
      return true;
    }

    BOOST_ASSERT_MSG(last_finalized_number >= block_header.slot,
                     "Target block must be finalized");

    // TODO This is case of change authority and we need to save justification
    // if (consensus::grandpa::HasAuthoritySetChange{block_header}) {
    //   return true;
    // }
    if (block_header.slot % 512 == 0) { // TODO slot or calculate depths?
      return true;
    }
    return false;
  }

}  // namespace kagome::blockchain
