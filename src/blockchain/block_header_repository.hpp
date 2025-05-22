/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <optional>

#include <qtils/byte_arr.hpp>
// #include "common/visitor.hpp"
// #include <qtils/outcome.hpp>
#include "jam_types/block_header.hpp"
// #include "primitives/block_id.hpp"

namespace jam::blockchain {

  /**
   * Status of a block
   */
  enum class BlockStatus : uint8_t {
    InChain,
    Unknown,
  };

  /**
   * An interface to a storage with block headers that provides several
   * convenience methods, such as getting bloch number by its hash and vice
   * versa or getting a block status
   */
  class BlockHeaderRepository {
   public:
    virtual ~BlockHeaderRepository() = default;

    /**
     * @return the number of the block with the provided {@param block_hash}
     * in case one is in the storage or an error
     */
    virtual outcome::result<BlockNumber> getNumberByHash(
        const BlockHash &block_hash) const = 0;

    // /**
    //  * @param block_number - the number of a block, contained in a block
    //  header
    //  * @return the hash of the block with the provided number in case one is
    //  * in the storage or an error
    //  */
    // virtual outcome::result<BlockHash> getHashByNumber(
    //     BlockNumber block_number) const = 0;

    /**
     * @return block header with corresponding {@param block_hash} or an error
     */
    [[nodiscard]] virtual outcome::result<BlockHeader> getBlockHeader(
        const BlockHash &block_hash) const = 0;

    // /**
    //  * @return block header with corresponding {@param block_hash} or a none
    //  * optional if the corresponding block header is not in storage or a
    //  * storage error
    //  */
    // virtual outcome::result<std::optional<BlockHeader>>
    // tryGetBlockHeader(const BlockHash &block_hash) const = 0;

    // /**
    //  * @param id of a block which number is returned
    //  * @return block number or a none optional if the corresponding block
    //  * header is not in storage or a storage error
    //  */
    // outcome::result<BlockNumber> getNumberById(
    //     const BlockId &block_id) const {
    //   return visit_in_place(
    //       block_id,
    //       [](const BlockNumber &block_number) {
    //         return block_number;
    //       },
    //       [this](const BlockHash &block_hash) {
    //         return getNumberByHash(block_hash);
    //       });
    // }
    //
    // /**
    //  * @param id of a block which hash is returned
    //  * @return block hash or a none optional if the corresponding block
    //  * header is not in storage or a storage error
    //  */
    // outcome::result<BlockHash> getHashById(
    //     const BlockId &id) const {
    //   return visit_in_place(
    //       id,
    //       [this](const BlockNumber &n) {
    //         return getHashByNumber(n);
    //       },
    //       [](const BlockHash &hash) { return hash; });
    // }
  };

}  // namespace jam::blockchain
