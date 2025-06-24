/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <crypto/hasher.hpp>
#include <jam_types/block_index.hpp>
#include <jam_types/epoch_mark.hpp>
#include <jam_types/types.hpp>
#include <utils/custom_equality.hpp>

namespace jam {

  /**
   * @struct BlockHeader represents header of a block
   */
  struct BlockHeader {
    /// Hp - parent block header hash
    HeaderHash parent;
    /// Hr - prior state root
    StateRoot parent_state_root;
    /// Hx - exctrinsic hash
    OpaqueHash extrinsic_hash;
    /// Ht - time-slot index
    TimeSlot slot;
    /// He - epoch marker
    std::optional<EpochMark> epoch_mark;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    /// Hw - winning-tickets marker
    std::optional<TicketsMark> tickets_mark;
    /// Ho - offenders marker
    OffendersMark offenders_mark;
#pragma GCC diagnostic pop
    /// Hi - Bandersnatch block author index
    ValidatorIndex author_index;
    /// Hv - the entropy-yielding vrf signature
    BandersnatchVrfSignature entropy_source;
    /// Hs - block seal
    BandersnatchVrfSignature seal;

    /// Block hash if calculated
    mutable std::optional<HeaderHash> hash_opt{};

    CUSTOM_EQUALITY(BlockHeader,
                    parent,
                    parent_state_root,
                    extrinsic_hash,
                    slot,
                    epoch_mark,
                    tickets_mark,
                    offenders_mark,
                    author_index,
                    entropy_source,
                    seal);
    SCALE_CUSTOM_DECOMPOSITION(BlockHeader,
                               parent,
                               parent_state_root,
                               extrinsic_hash,
                               slot,
                               epoch_mark,
                               tickets_mark,
                               offenders_mark,
                               author_index,
                               entropy_source,
                               seal);

    const HeaderHash &hash() const {
      BOOST_ASSERT_MSG(hash_opt.has_value(),
                       "Hash must be calculated and saved before that");
      return hash_opt.value();
    }

    void updateHash(const crypto::Hasher &hasher) const {
      auto enc_res = encode(*this);
      BOOST_ASSERT_MSG(enc_res.has_value(),
                       "Header should be encoded errorless");
      hash_opt.emplace(hasher.blake2b_256(enc_res.value()));
    }

    BlockIndex index() const {
      return {slot, hash()};
    }
  };

  struct UnsealedBlockHeader : private BlockHeader {
    UnsealedBlockHeader() = delete;

    using BlockHeader::author_index;
    using BlockHeader::entropy_source;
    using BlockHeader::epoch_mark;
    using BlockHeader::extrinsic_hash;
    using BlockHeader::offenders_mark;
    using BlockHeader::parent;
    using BlockHeader::parent_state_root;
    using BlockHeader::slot;
    using BlockHeader::tickets_mark;

    CUSTOM_EQUALITY(UnsealedBlockHeader,
                    parent,
                    parent_state_root,
                    extrinsic_hash,
                    slot,
                    epoch_mark,
                    tickets_mark,
                    offenders_mark,
                    author_index,
                    entropy_source);
    SCALE_CUSTOM_DECOMPOSITION(UnsealedBlockHeader,
                               parent,
                               parent_state_root,
                               extrinsic_hash,
                               slot,
                               epoch_mark,
                               tickets_mark,
                               offenders_mark,
                               author_index,
                               entropy_source);
  };

  inline void calculateBlockHash(const BlockHeader &header,
                                 const crypto::Hasher &hasher) {
    header.updateHash(hasher);
  }

}  // namespace jam
