/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jam_types/common-types.hpp>
#include <jam_types/tv_proxy.hpp>

#include "tests/mock/crypto/hasher_mock.hpp"

#define AS_IN_TEST_VECTORS(TYPE) \
  static_assert(std::same_as<TYPE, test_vectors::TYPE>)

#define HAS_PROXY_TEST_VECTORS(TYPE) \
  template <>                        \
  class ::jam::test_vectors::Proxy<TYPE, jam::test_vectors::TYPE>;

// clang-format off
#define TEMPORARY_TAKE_TYPE_FROM_TEST_VECTORS(TYPE)                   \
  using TYPE [[deprecated(                                            \
      #TYPE " is type from test-vector. Implement native type")]] =   \
      jam::test_vectors::TYPE;                                        \
  _Pragma("GCC diagnostic push")                                      \
  _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")     \
  AS_IN_TEST_VECTORS(TYPE);                                           \
  HAS_PROXY_TEST_VECTORS(TYPE);                                       \
  _Pragma("GCC diagnostic pop") /* NOLINT */
// clang-format on

#define TEMPORARY_TYPE_ALIAS(ALIAS, ...)                            \
  using ALIAS [[deprecated(#ALIAS " is alias of " #__VA_ARGS__ ". " \
                                  "Implement native type")]] = __VA_ARGS__

namespace jam {

  using TimeSlot = uint32_t;
  AS_IN_TEST_VECTORS(TimeSlot);

  using ValidatorIndex = uint16_t;
  AS_IN_TEST_VECTORS(ValidatorIndex);

  using OpaqueHash = qtils::ByteArr<32>;
  AS_IN_TEST_VECTORS(OpaqueHash);

  using Entropy = qtils::ByteArr<32>;
  AS_IN_TEST_VECTORS(Entropy);

  using HeaderHash = qtils::ByteArr<32>;
  AS_IN_TEST_VECTORS(HeaderHash);

  using StateRoot = qtils::ByteArr<32>;
  AS_IN_TEST_VECTORS(StateRoot);

  using BandersnatchPublic = qtils::ByteArr<32>;
  AS_IN_TEST_VECTORS(BandersnatchPublic);

  using BandersnatchVrfSignature = qtils::ByteArr<96>;
  AS_IN_TEST_VECTORS(BandersnatchVrfSignature);

  // header
  TEMPORARY_TAKE_TYPE_FROM_TEST_VECTORS(OffendersMark);
  TEMPORARY_TAKE_TYPE_FROM_TEST_VECTORS(TicketsMark);

  // extrinsic
  TEMPORARY_TAKE_TYPE_FROM_TEST_VECTORS(AssurancesExtrinsic);
  TEMPORARY_TAKE_TYPE_FROM_TEST_VECTORS(DisputesExtrinsic);
  TEMPORARY_TAKE_TYPE_FROM_TEST_VECTORS(GuaranteesExtrinsic);
  TEMPORARY_TAKE_TYPE_FROM_TEST_VECTORS(PreimagesExtrinsic);
  TEMPORARY_TAKE_TYPE_FROM_TEST_VECTORS(TicketsExtrinsic);

  using BlockHash = HeaderHash;

}  // namespace jam
