/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <gmock/gmock.h>

#include "blockchain/impl/justification_storage_policy.hpp"

namespace jam::blockchain {

  class JustificationStoragePolicyMock : public JustificationStoragePolicy {
   public:
    MOCK_METHOD(outcome::result<bool>,
                shouldStoreFor,
                (const BlockHeader &header,
                 BlockNumber last_finalized_number),
                (const, override));

   private:
  };

}  // namespace kagome::blockchain
