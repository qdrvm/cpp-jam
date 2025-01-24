/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <algorithm>
#include <ranges>
#include <set>
#include <unordered_map>

#include <TODO_qtils/cxx23/ranges/contains.hpp>
#include <jam/bandersnatch.hpp>
#include <src_/jam/tagged.hpp>
#include <test-vectors/common-scale.hpp>
#include <test-vectors/common-types.hpp>
#include <test-vectors/common.hpp>
#include <test-vectors/config-full.hpp>

namespace jam::safrole {
  namespace types = jam::test_vectors;

  using BandersnatchSignature = decltype(types::TicketEnvelope::signature);

  // [GP 0.4.5 G 339]
  // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/bandersnatch.tex#L9
  // [GP 0.4.5 G 342]
  // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/bandersnatch.tex#L17
  inline std::optional<types::OpaqueHash> banderout(
      const BandersnatchSignature &signature) {
    return bandersnatch::output(signature);
  }

  struct TicketBodyLess {
    bool operator()(const types::TicketBody &l,
                    const types::TicketBody &r) const {
      return l.id < r.id;
    }
  };

  using GammaA = decltype(types::safrole::State::gamma_a);
  using GammaZ = decltype(types::safrole::State::gamma_z);
  using BandersnatchKeys = decltype(types::EpochMark::validators);

  inline auto &ring_ctx(const types::Config &config) {
    static std::unordered_map<uint32_t, bandersnatch::Ring> map;
    auto &n = config.validators_count;
    auto it = map.find(n);
    if (it == map.end()) {
      it = map.emplace(n, bandersnatch::Ring{n}).first;
    }
    return it->second;
  }

  // [GP 0.4.5 I.4.5]
  // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/definitions.tex#L303
  // clang-format off
  constexpr qtils::BytesN<15> X_T = {'j','a','m','_','t','i','c','k','e','t','_','s','e','a','l'};
  // clang-format on

  // The maximum number of tickets which may be submitted in a single extrinsic.
  // [GP 0.4.5 I.4.4]
  // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/definitions.tex#L269
  constexpr uint32_t K = 16;

  inline BandersnatchKeys bandersnatch_keys(
      const types::ValidatorsData &validators) {
    BandersnatchKeys keys;
    keys.reserve(validators.size());
    for (auto &validator : validators) {
      keys.emplace_back(validator.bandersnatch);
    }
    return keys;
  }

  // [GP 0.4.5 G 340]
  // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/bandersnatch.tex#L15
  inline GammaZ mathcal_O(const types::Config &config,
                          const BandersnatchKeys &pks) {
    return ring_ctx(config).commitment(pks).value();
  }

  // [GP 0.4.5 G 341]
  // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/bandersnatch.tex#L16
  inline std::optional<types::OpaqueHash> bandersnatch(
      const types::Config &config,
      const GammaZ &gamma_z,
      qtils::BytesIn input,
      const BandersnatchSignature &signature) {
    return ring_ctx(config).verifier(gamma_z).verify(input, signature);
  }

  // [GP 0.4.5 6.1 47]
  // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/safrole.tex#L30
  struct Epoch {
    uint32_t epoch, phase;
    Epoch(uint32_t E, uint32_t slot) : epoch{slot / E}, phase{slot % E} {}
  };

  /**
   * Given state and input, derive next state and output.
   */
  inline std::pair<types::safrole::State, types::safrole::Output> transition(
      const types::Config &config,
      const types::safrole::State &state,
      const types::safrole::Input &input) {
    /// The length of an epoch in timeslots.
    // [GP 0.4.5 I.4.4]
    // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/definitions.tex#L260
    const auto E = config.epoch_length;

    // The number of ticket entries per validator.
    // [GP 0.4.5 I.4.4]
    // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/definitions.tex#L271
    const uint32_t N = config.tickets_per_validator;

    /// The number of slots into an epoch at which ticket-submission ends.
    // [GP 0.4.5 I.4.4]
    // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/definitions.tex#L287
    const auto Y = E * 5 / 6;

    const auto &[
        // [H_t] Current time slot as found within the block header.
        slot,
        // [Y(H_v)] Per block entropy generated using per block entropy source.
        entropy,
        // [E_T] Tickets extrinsic.
        extrinsic] = input;

    const auto &[
        // [τ] Prior most recent block's timeslot. Mutated to τ'.
        tau,
        // [η] Prior entropy buffer. Mutated to η'.
        eta,
        // [λ] Prior previous epoch validator keys and metadata. Mutated to λ'.
        lambda,
        // [κ] Prior current epoch validator keys and metadata. Mutated to κ'.
        kappa,
        // [γ_k] Prior next epoch validator keys and metadata. Mutated to γ'_k.
        gamma_k,
        // [ι] Prior scheduled validator keys and metadata. Mutated to ι'.
        iota,
        // [γ_a] Prior sealing-key contest ticket accumulator. Mutated to γ'_a.
        gamma_a,
        // [γ_s] Prior sealing-key series of the current epoch. Mutated to γ'_s.
        gamma_s,
        // [γ_z] Prior Bandersnatch ring commitment. Mutated to γ'_z.
        gamma_z,
        // [ψ'_o] Posterior offenders sequence.
        post_offenders] = state;

    // η0 defines the state of the randomness accumulator to which the provably
    // random output of the vrf, the signature over some unbiasable input, is
    // combined each block. η1, η2 and η3 meanwhile retain the state of this
    // accumulator at the end of the three most recently ended epochs in order.
    const auto &[eta_0, eta_1, eta_2, eta_3] = eta;
    using Error = types::safrole::ErrorCode;
    const auto error = [&](Error error) {
      return std::make_pair(state, types::safrole::Output{error});
    };

    // A block may only be regarded as valid once the time-slot index Ht is in
    // the past.
    // It's always strictly greater than that of its parent.
    // [GP 0.4.5 5 42]
    // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/header.tex#L29
    if (slot <= state.tau) {
      return error(Error::bad_slot);
    }

    // The most recent block’s slot index, which we transition to the slot index
    // as defined in the block’s header.
    // [GP 0.4.5 6.1 46]
    // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/safrole.tex#L25
    const auto tau_tick = slot;

    // [GP 0.4.5 6.1 47]
    // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/safrole.tex#L30
    const Epoch epoch{E, state.tau};
    const Epoch epoch_tick{E, tau_tick};

    // the prior’s epoch index and slot phase index within that epoch
    const auto &[e, m] = epoch;

    // epoch index and slot phase index are the corresponding values for the
    // present block
    const auto &[e_tick, m_tick] = epoch_tick;

    const auto change_epoch = e_tick != e;

    // [GP 0.4.5 6.7 75]
    // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/safrole.tex#L251
    if (extrinsic.size() > K) {
      throw std::logic_error("not covered by test vectors");
    }
    if (m_tick >= Y and not extrinsic.empty()) {
      return error(Error::unexpected_ticket);
    }

    // [GP 0.4.5 6.3 59]
    // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/safrole.tex#L101
    const auto phi = [&](const types::ValidatorsData &k) {
      types::ValidatorsData k_tick;
      for (auto &validator : k) {
        k_tick.emplace_back(
            qtils::cxx23::ranges::contains(post_offenders, validator.ed25519)
                ? types::ValidatorData{}
                : validator);
      }
      return k_tick;
    };
    // [GP 0.4.5 6.3 58]
    // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/safrole.tex#L97
    const auto [gamma_tick_k, kappa_tick, lambda_tick, gamma_tick_z] =
        change_epoch ? [&](const types::ValidatorsData &gamma_tick_k) {
          return std::tuple{
              gamma_tick_k,
              gamma_k,
              kappa,
              mathcal_O(config, bandersnatch_keys(gamma_tick_k)),
          };
        }(phi(iota))
                     : std::tuple{gamma_k, kappa, lambda, gamma_z};

    // [GP 0.4.5 6.4 67]
    // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/safrole.tex#L147
    // η0 defines the state of the randomness accumulator to which the provably
    // random output of the vrf, the signature over some unbiasable input, is
    // combined each block. η1, η2 and η3 meanwhile retain the state of this
    // accumulator at the end of the three most recently ended epochs in order.
    const auto eta_tick_0 = mathcal_H(frown(eta_0, entropy));

    // [GP 0.4.5 6.4 68]
    // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/safrole.tex#L152
    // On an epoch transition (identified as the condition e′ > e), we therefore
    // rotate the accumulator value into the history η1, η2 and η3:
    const auto [eta_tick_1, eta_tick_2, eta_tick_3] =
        change_epoch ? std::tuple{eta_0, eta_1, eta_2}
                     : std::tuple{eta_1, eta_2, eta_3};

    // The new ticket accumulator γ′ a is constructed by
    // merging new tickets into the previous accumulator value
    // (or the empty sequence if it is a new epoch)
    // [GP 0.5.2 6.7 (6.34)]
    std::set<types::TicketBody, TicketBodyLess> gamma_tick_a;
    if (not change_epoch) {
      gamma_tick_a.insert(gamma_a.begin(), gamma_a.end());
    }

    std::optional<test_vectors::TicketBody> prev_ticket;
    for (const auto &ticket_envelope : extrinsic) {
      auto &[attempt, ticket_proof] = ticket_envelope;

      auto m = frown(X_T, doubleplus(eta_tick_2, attempt));
      const auto ticket_id_opt = bandersnatch(config, gamma_z, m, ticket_proof);
      if (not ticket_id_opt.has_value()) {
        return error(Error::bad_ticket_proof);
      }
      const auto &ticket_id = ticket_id_opt.value();

      types::TicketBody ticket{
          .id = ticket_id,
          .attempt = attempt,
      };

      // We define the extrinsic as a sequence of proofs of valid tickets,
      // each of which is a tuple of an entry index (a natural number less
      // than N) and a proof of ticket validity
      // [GP 0.5.2 6.7 (6.29)]
      if (attempt >= N) {
        return error(Error::bad_ticket_attempt);
      }

      // Duplicate identifiers are neve allowed lest a validator submit the
      // same ticket multiple times
      // [GP 0.5.2 6.7 (6.33)]
      if (qtils::cxx23::ranges::contains(gamma_tick_a, ticket)) {
        return error(Error::duplicate_ticket);
      }

      // The tickets submitted via the extrinsic must already have been placed
      // in order of their implied identifier.
      // [GP 0.5.2 6.7 (6.32)]
      if (prev_ticket and TicketBodyLess{}(ticket, *prev_ticket)) {
        return error(Error::bad_ticket_order);
      }
      prev_ticket = ticket;

      // [GP 0.5.2 6.7 (6.34)]
      gamma_tick_a.emplace(ticket);

      // The maximum size of the ticket accumulator is E. On each block, the
      // accumulator becomes the lowest items of the sorted union of tickets
      // from prior accumulator γa and the submitted tickets.
      if (gamma_tick_a.size() > E) {
        gamma_tick_a.erase(std::prev(gamma_tick_a.end()));
      }
    }

    // [GP 0.4.5 6.5 70]
    // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/safrole.tex#L182
    const auto Z = [&](const GammaA &s) {
      using TicketsBodies =
          std::variant_alternative_t<0, types::TicketsOrKeys::type>;
      TicketsBodies tickets;
      if (s.size() != E) {
        throw std::logic_error{"Z"};
      }
      auto it1 = s.begin(), it2 = std::prev(s.end());
      auto odd = true;
      for (uint32_t i = 0; i < TicketsBodies::configSize(config); ++i) {
        tickets.emplace_back(odd ? *it1 : *it2);
        if (odd) {
          ++it1;
        } else {
          --it2;
        }
        odd = not odd;
      }
      return tickets;
    };

    // [GP 0.4.5 6.5 71]
    // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/safrole.tex#L190
    const auto F = [&](const types::OpaqueHash &r,
                       const types::ValidatorsData &k) {
      using EpochKeys =
          std::variant_alternative_t<1, types::TicketsOrKeys::type>;
      EpochKeys keys;
      for (uint32_t i = 0; i < E; ++i) {
        keys.emplace_back(
            circlearrowleft(
                k, de(first_bytes<4>(mathcal_H(frown(r, mathcal_E<4>(i))))))
                .bandersnatch);
      }
      return keys;
    };
    // [GP 0.4.5 6.5 69]
    // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/safrole.tex#L173
    const auto gamma_tick_s =
        e_tick == e + 1 && m >= Y && gamma_a.size() == E
            ? types::TicketsOrKeys{Z(gamma_a)}
        : e_tick == e ? gamma_s
                      : types::TicketsOrKeys{F(eta_tick_2, kappa_tick)};

    // the header’s epoch marker He is either empty or, if the block is the
    // first in a new epoch, then a tuple of the next and current epoch
    // randomness, along with a sequence of Bandersnatch keys defining the
    // Bandersnatch validator keys (kb) beginning in the next epoch.
    // [GP 0.5.2 6.6 (6.27)]
    std::optional<types::safrole::EpochMark> epoch_mark{};
    if (change_epoch) {
      epoch_mark =
          types::EpochMark{.entropy = eta_tick_1,
                           .tickets_entropy = eta_tick_2,
                           .validators = bandersnatch_keys(gamma_tick_k)};
    };

    return {
        types::safrole::State{
            .tau = tau_tick,
            .eta = {eta_tick_0, eta_tick_1, eta_tick_2, eta_tick_3},
            .lambda = lambda_tick,
            .kappa = kappa_tick,
            .gamma_k = gamma_tick_k,
            // TODO(turuslan): #3, wait for test vectors
            .iota = iota,
            .gamma_a = {gamma_tick_a.begin(), gamma_tick_a.end()},
            .gamma_s = gamma_tick_s,
            .gamma_z = gamma_tick_z,
            .post_offenders = post_offenders,
        },
        types::safrole::Output{types::safrole::OutputData{
            .epoch_mark = epoch_mark,
            // [GP 0.4.5 6.6 73]
            // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/safrole.tex#L224
            .tickets_mark =
                e_tick == e && m < Y && Y <= m_tick && gamma_a.size() == E
                    ? std::make_optional(types::TicketsMark{Z(gamma_a)})
                    : std::nullopt,
        }},
    };
  }
}  // namespace jam::safrole
