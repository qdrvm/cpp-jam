/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <gmock/gmock.h>

#include "blockchain/block_storage.hpp"

namespace jam::blockchain {

  class BlockStorageMock : public BlockStorage {
   public:
    MOCK_METHOD(outcome::result<void>,
                setBlockTreeLeaves,
                (std::vector<BlockHash>),
                (override));

    MOCK_METHOD(outcome::result<std::vector<BlockHash>>,
                getBlockTreeLeaves,
                (),
                (const, override));

    MOCK_METHOD(outcome::result<BlockIndex>,
                getLastFinalized,
                (),
                (const, override));

    MOCK_METHOD(outcome::result<void>,
                assignHashToSlot,
                (const BlockIndex &),
                (override));

    MOCK_METHOD(outcome::result<void>,
                deassignHashToSlot,
                (const BlockIndex &),
                (override));

    MOCK_METHOD(outcome::result<std::vector<BlockHash>>,
                getBlockHash,
                (TimeSlot),
                (const, override));
    // MOCK_METHOD(outcome::result<std::optional<BlockHash>>,
    //             getBlockHash,
    //             (const BlockId &),
    //             (const, override));

    MOCK_METHOD(outcome::result<bool>,
                hasBlockHeader,
                (const BlockHash &),
                (const, override));

    MOCK_METHOD(outcome::result<BlockHash>,
                putBlockHeader,
                (const BlockHeader &),
                (override));

    MOCK_METHOD(outcome::result<BlockHeader>,
                getBlockHeader,
                (const BlockHash &),
                (const, override));

    MOCK_METHOD(outcome::result<std::optional<BlockHeader>>,
                tryGetBlockHeader,
                (const BlockHash &),
                (const, override));

    MOCK_METHOD(outcome::result<void>,
                putExtrinsic,
                (const BlockHash &, const Extrinsic &),
                (override));

    MOCK_METHOD(outcome::result<std::optional<Extrinsic>>,
                getExtrinsic,
                (const BlockHash &),
                (const, override));

    MOCK_METHOD(outcome::result<void>,
                removeExtrinsic,
                (const BlockHash &),
                (override));

    MOCK_METHOD(outcome::result<void>,
                putJustification,
                (const Justification &, const BlockHash &),
                (override));

    MOCK_METHOD(outcome::result<std::optional<Justification>>,
                getJustification,
                (const BlockHash &),
                (const, override));

    MOCK_METHOD(outcome::result<void>,
                removeJustification,
                (const BlockHash &),
                (override));

    MOCK_METHOD(outcome::result<BlockHash>,
                putBlock,
                (const Block &),
                (override));

    MOCK_METHOD(outcome::result<std::optional<BlockData>>,
                getBlockData,
                (const BlockHash &),
                (const, override));

    MOCK_METHOD(outcome::result<void>,
                removeBlock,
                (const BlockHash &),
                (override));
  };

}  // namespace jam::blockchain
