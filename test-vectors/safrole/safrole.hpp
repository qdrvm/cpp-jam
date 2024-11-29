/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <algorithm>
#include <ranges>
#include <unordered_map>

#include <unordered_map>

#include <jam/bandersnatch.hpp>
#include <src/jam/tagged.hpp>
#include <src/jam/variant_alternative.hpp>
#include <test-vectors/common-scale.hpp>
#include <test-vectors/common-types.hpp>
#include <test-vectors/common.hpp>

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

  // The number of ticket entries per validator.
  // [GP 0.4.5 I.4.4]
  // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/definitions.tex#L271
  constexpr uint32_t N = 2;

  // The maximum number of tickets which may be submitted in a single extrinsic.
  // [GP 0.4.5 I.4.4]
  // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/definitions.tex#L269
  constexpr uint32_t K = 16;

  inline BandersnatchKeys bandersnatch_keys(
      const types::ValidatorsData &validators) {
    BandersnatchKeys keys;
    for (auto &validator : validators.v) {
      keys.v.emplace_back(validator.bandersnatch);
    }
    return keys;
  }

  // [GP 0.4.5 G 340]
  // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/bandersnatch.tex#L15
  inline GammaZ mathcal_O(const types::Config &config,
                          const BandersnatchKeys &pks) {
    return ring_ctx(config).commitment(pks.v).value();
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

    /// The number of slots into an epoch at which ticket-submission ends.
    // [GP 0.4.5 I.4.4]
    // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/definitions.tex#L287
    const auto Y = E * 5 / 6;

    /// H_t - a time-slot index
    const auto &[H_t, banderout_H_v, E_T] = input;
    const auto &[tau,
                 eta,
                 lambda,
                 kappa,
                 gamma_k,
                 iota,
                 gamma_a,
                 gamma_s,
                 gamma_z,
                 post_offenders] = state;
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
    if (H_t <= state.tau) {
      return error(Error::bad_slot);
    }

    // The most recent block’s slot index, which we transition to the slot index
    // as defined in the block’s header.
    // [GP 0.4.5 6.1 46]
    // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/safrole.tex#L25
    const auto tau_tick = H_t;

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
    if (E_T.size() > K) {
      throw std::logic_error("not covered by test vectors");
    }
    if (m_tick >= Y and not E_T.empty()) {
      return std::make_pair(state,
                            types::safrole::Output{Error::unexpected_ticket});
    }

    // [GP 0.4.5 6.3 59]
    // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/safrole.tex#L101
    const auto phi = [&](const types::ValidatorsData &k) {
      types::ValidatorsData k_tick;
      for (auto &validator : k.v) {
        k_tick.v.emplace_back(
            std::ranges::find(post_offenders, validator.ed25519)
                    != post_offenders.end()
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
    const auto eta_tick_0 = mathcal_H(frown(eta_0, banderout_H_v));

    // [GP 0.4.5 6.4 68]
    // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/safrole.tex#L152
    const auto [eta_tick_1, eta_tick_2, eta_tick_3] =
        change_epoch ? std::tuple{eta_0, eta_1, eta_2}
                     : std::tuple{eta_1, eta_2, eta_3};

    std::vector<types::TicketBody> n;
    for (auto &[r, p] : E_T) {
      // [GP 0.4.5 6.7 74]
      // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/safrole.tex#L250
      if (r >= N) {
        return error(Error::bad_ticket_attempt);
      }
      const auto m = frown(X_T, doubleplus(eta_tick_2, r));
      const auto y = bandersnatch(config, gamma_z, m, p);
      if (not y) {
        return error(Error::bad_ticket_proof);
      }
      // [GP 0.4.5 6.7 76]
      // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/safrole.tex#L259
      n.emplace_back(types::TicketBody{*y, r});
    }
    // [GP 0.4.5 6.7 77]
    // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/safrole.tex#L264
    if (not std::is_sorted(n.begin(), n.end(), TicketBodyLess{})) {
      return error(Error::bad_ticket_order);
    }

    // [GP 0.4.5 6.7 79]
    // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/safrole.tex#L271
    std::vector<types::TicketBody> gamma_tick_a;
    if (change_epoch) {
      gamma_tick_a = n;
    } else {
      std::ranges::set_union(
          n, gamma_a, std::back_inserter(gamma_tick_a), TicketBodyLess{});
      // [GP 0.4.5 6.7 78]
      // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/safrole.tex#L265
      if (gamma_tick_a.size() != gamma_a.size() + n.size()) {
        return error(Error::duplicate_ticket);
      }
    }
    if (gamma_tick_a.size() > E) {
      gamma_tick_a.resize(E);
    }

    // [GP 0.4.5 6.5 70]
    // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/safrole.tex#L182
    const auto Z = [&](const GammaA &s) {
      using TicketsBodies =
          variant_alternative_t<0, types::TicketsOrKeys::type>;
      TicketsBodies tickets;
      if (s.size() != E) {
        throw std::logic_error{"Z"};
      }
      auto it1 = s.begin(), it2 = std::prev(s.end());
      auto odd = true;
      for (uint32_t i = 0; i < TicketsBodies::configSize(config); ++i) {
        tickets.v.emplace_back(odd ? *it1 : *it2);
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
      using EpochKeys = variant_alternative_t<1, types::TicketsOrKeys::type>;
      EpochKeys keys;
      for (uint32_t i = 0; i < E; ++i) {
        keys.v.emplace_back(
            circlearrowleft(
                k.v, de(first_bytes<4>(mathcal_H(frown(r, mathcal_E<4>(i))))))
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

    types::safrole::Output output;
    return {
        types::safrole::State{
            .tau = tau_tick,
            .eta = {eta_tick_0, eta_tick_1, eta_tick_2, eta_tick_3},
            .lambda = lambda_tick,
            .kappa = kappa_tick,
            .gamma_k = gamma_tick_k,
            // TODO(turuslan): #3, wait for test vectors
            .iota = iota,
            .gamma_a = gamma_tick_a,
            .gamma_s = gamma_tick_s,
            .gamma_z = gamma_tick_z,
        },
        types::safrole::Output{types::safrole::OutputData{
            // [GP 0.4.5 6.6 72]
            // https://github.com/gavofyork/graypaper/blob/v0.4.5/text/safrole.tex#L216
            .epoch_mark = change_epoch ? std::make_optional(
                              types::EpochMark{eta_tick_1,
                                               eta_tick_1,  // ???
                                               bandersnatch_keys(gamma_tick_k)})
                                       : std::nullopt,
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
