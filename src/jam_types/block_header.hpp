/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <crypto/hasher.hpp>
#include <jam_types/types.tmp.hpp>
#include <scale/jam_scale.hpp>
#include <utils/custom_equality.hpp>

namespace jam {

  using test_vectors::BandersnatchVrfSignature;
  using test_vectors::EpochMark;
  using test_vectors::HeaderHash;
  using test_vectors::OffendersMark;
  using test_vectors::OpaqueHash;
  using test_vectors::StateRoot;
  using test_vectors::TicketsMark;
  using test_vectors::TimeSlot;
  using test_vectors::ValidatorIndex;

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
    /// Hw - winning-tickets marker
    std::optional<TicketsMark> tickets_mark;
    /// Ho - offenders marker
    OffendersMark offenders_mark;
    /// Hi - Bandersnatch block author index
    ValidatorIndex author_index;
    /// Hv - the entropy-yielding vrf signature
    BandersnatchVrfSignature entropy_source;
    /// Hs - block seal
    BandersnatchVrfSignature seal;

    /// Block hash if calculated
    mutable std::optional<BlockHash> hash_opt{};

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

    const BlockHash &hash() const {
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

    BlockInfo index() const {
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
