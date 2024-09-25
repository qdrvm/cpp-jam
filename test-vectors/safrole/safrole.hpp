/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jam_crust.h>

#include "../common.hpp"
#include "types.hpp"

namespace jam::safrole {
  namespace generic = jam::test_vectors_safrole::generic;
  using BandersnatchSignature = decltype(generic::TicketEnvelope::signature);

  template <typename types>
  struct Generic;

  // [GP 0.3.6 G 307]
  // [GP 0.3.6 G 310]
  inline std::optional<generic::OpaqueHash> banderout(
      const BandersnatchSignature &signature) {
    generic::OpaqueHash out;
    if (not jam_vrf_output(signature.data(), out.data())) {
      return std::nullopt;
    }
    return out;
  }

  struct TicketBodyLess {
    bool operator()(
        const generic::TicketBody &l, const generic::TicketBody &r) const {
      return l.id < r.id;
    }
  };
}  // namespace jam::safrole

template <typename types>
struct jam::safrole::Generic {
  using GammaA = decltype(types::State::gamma_a);
  using GammaZ = decltype(types::State::gamma_z);
  using BandersnatchKeys = decltype(types::EpochMark::validators);

  static auto *ring_ctx() {
    static auto ctx = jam_ring_new(types::validators_count);
    return ctx;
  }

  // [GP 0.3.6 I.4.5]
  // clang-format off
  static constexpr qtils::BytesN<15> X_T = {'j','a','m','_','t','i','c','k','e','t','_','s','e','a','l'};
  // clang-format on

  // [GP 0.3.6 I.4.4]
  static constexpr auto E = types::epoch_length;

  // [GP 0.3.6 I.4.4]
  static constexpr uint32_t N = 2;
  // [GP 0.3.6 I.4.4]
  static constexpr uint32_t K = 16;

  // [GP 0.3.6 I.4.4]
  static constexpr uint32_t Y = E * 5 / 6;

  static BandersnatchKeys bandersnatch_keys(
      const types::ValidatorsData &validators) {
    BandersnatchKeys keys;
    for (size_t i = 0; i < keys.size(); ++i) {
      keys[i] = validators[i].bandersnatch;
    }
    return keys;
  }

  // [GP 0.3.6 G 308]
  static GammaZ mathcal_O(const BandersnatchKeys &pks) {
    if (sizeof(pks) != pks.size() * sizeof(pks[0])) {
      // BUG: static_assert
      throw std::logic_error{"mathcal_O"};
    }
    GammaZ out;
    if (not jam_ring_commitment(
            ring_ctx(), pks[0].data(), sizeof(pks), out.data())) {
      throw std::logic_error("mathcal_O");
    }
    return out;
  }

  // [GP 0.3.6 G 309]
  static std::optional<typename types::OpaqueHash> bandersnatch(
      const GammaZ &gamma_z,
      qtils::BytesIn input,
      const BandersnatchSignature &signature) {
    typename types::OpaqueHash out;
    std::unique_ptr<JamRingVerifier, decltype(&jam_ring_verifier_drop)>
        verifier{
            jam_ring_verifier_new(ring_ctx(), gamma_z.data()),
            &jam_ring_verifier_drop,
        };
    if (not jam_ring_verifier_verify(verifier.get(),
            input.data(),
            input.size(),
            signature.data(),
            out.data())) {
      return std::nullopt;
    }
    return out;
  }

  // [GP 0.3.6 6.1 46]
  struct Epoch {
    uint32_t epoch, phase;
    Epoch(uint32_t slot) : epoch{slot / E}, phase{slot % E} {}
  };

  /**
   * Given state and input, derive next state and output.
   */
  static std::pair<typename types::State, typename types::Output> transition(
      const types::State &state, const types::Input &input) {
    const auto &[H_t, banderout_H_v, E_T] = input;
    const auto
        &[tau, eta, lambda, kappa, gamma_k, iota, gamma_a, gamma_s, gamma_z] =
            state;
    const auto &[eta_0, eta_1, eta_2, eta_3] = eta;
    using Error = types::CustomErrorCode;
    const auto error = [&](Error error) {
      return std::make_pair(state, typename types::Output{error});
    };

    // [GP 0.3.6 5 41]
    if (H_t <= state.tau) {
      return error(Error::bad_slot);
    }

    // [GP 0.3.6 6.1 45]
    const auto tau_tick = H_t;

    // [GP 0.3.6 6.1 46]
    const Epoch epoch{state.tau}, epoch_tick{tau_tick};
    const auto &[e, m] = epoch;
    const auto &[e_tick, m_tick] = epoch_tick;
    const auto change_epoch = e_tick != e;

    // [GP 0.3.6 6.7 74]
    if (E_T.size() > K) {
      throw std::logic_error("not covered by test vectors");
    }
    if (m_tick >= Y and not E_T.empty()) {
      return std::make_pair(
          state, typename types::Output{Error::unexpected_ticket});
    }

    // [GP 0.3.6 6.3 58]
    // TODO(turuslan): #3, wait for test vectors
    const auto phi = [](const types::ValidatorsData &k) { return k; };
    // [GP 0.3.6 6.3 57]
    const auto [gamma_tick_k, kappa_tick, lambda_tick, gamma_tick_z] = change_epoch ?
      [&](const types::ValidatorsData&gamma_tick_k) {
        return std::tuple{gamma_tick_k, gamma_k, kappa, mathcal_O(bandersnatch_keys(gamma_tick_k))};
      }(phi(iota)) :
      std::tuple{gamma_k,kappa,lambda,gamma_z};

    // [GP 0.3.6 6.4 66]
    const auto eta_tick_0 = mathcal_H(frown(eta_0, banderout_H_v));

    // [GP 0.3.6 6.4 67]
    const auto [eta_tick_1, eta_tick_2, eta_tick_3] = change_epoch
        ? std::tuple{eta_0, eta_1, eta_2}
        : std::tuple{eta_1, eta_2, eta_3};

    std::vector<typename types::TicketBody> n;
    for (auto &[r, p] : E_T) {
      // [GP 0.3.6 6.7 73]
      if (r >= N) {
        return error(Error::bad_ticket_attempt);
      }
      const auto m = frown(X_T, doubleplus(eta_tick_2, r));
      const auto y = bandersnatch(gamma_z, m, p);
      if (not y) {
        return error(Error::bad_ticket_proof);
      }
      // [GP 0.3.6 6.7 75]
      n.emplace_back(typename types::TicketBody{*y, r});
    }
    // [GP 0.3.6 6.7 76]
    if (not std::is_sorted(n.begin(), n.end(), TicketBodyLess{})) {
      return error(Error::bad_ticket_order);
    }

    // [GP 0.3.6 6.7 78]
    std::vector<typename types::TicketBody> gamma_tick_a;
    if (change_epoch) {
      gamma_tick_a = n;
    } else {
      std::set_union(n.begin(),
          n.end(),
          gamma_a.begin(),
          gamma_a.end(),
          std::back_inserter(gamma_tick_a),
          TicketBodyLess{});
      // [GP 0.3.6 6.7 77]
      if (gamma_tick_a.size() != gamma_a.size() + n.size()) {
        return error(Error::duplicate_ticket);
      }
    }
    if (gamma_tick_a.size() > E) {
      gamma_tick_a.resize(E);
    }

    // [GP 0.3.6 6.5 69]
    const auto Z = [](const GammaA &s) {
      typename types::TicketsBodies tickets;
      if (s.size() != E) {
        throw std::logic_error{"Z"};
      }
      auto it1 = s.begin(), it2 = std::prev(s.end());
      auto odd = true;
      for (auto &ticket : tickets) {
        ticket = odd ? *it1 : *it2;
        if (odd) {
          ++it1;
        } else {
          --it2;
        }
        odd = not odd;
      }
      return tickets;
    };
    // [GP 0.3.6 6.5 70]
    const auto F = [](const types::OpaqueHash &r,
                       const types::ValidatorsData &k) {
      typename types::EpochKeys keys;
      for (uint32_t i = 0; i < E; ++i) {
        keys[i] = circlearrowleft(
            k, de(first_bytes<4>(mathcal_H(frown(r, mathcal_E<4>(i))))))
                      .bandersnatch;
      }
      return keys;
    };
    // [GP 0.3.6 6.5 68]
    const auto gamma_tick_s = e_tick == e + 1 && m >= Y && gamma_a.size() == E
        ? typename types::TicketsOrKeys{Z(gamma_a)}
        : e_tick == e
        ? gamma_s
        : typename types::TicketsOrKeys{F(eta_tick_2, kappa_tick)};

    typename types::Output output;
    return {
        typename types::State{
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
        typename types::Output{typename types::OutputMarks{
            // [GP 0.3.6 6.6 71]
            .epoch_mark = change_epoch
                ? std::make_optional(typename types::EpochMark{
                    eta_tick_1, bandersnatch_keys(gamma_tick_k)})
                : std::nullopt,
            // [GP 0.3.6 6.6 72]
            .tickets_mark =
                e_tick == e && m < Y && Y <= m_tick && gamma_a.size() == E
                ? std::make_optional(typename types::TicketsMark{Z(gamma_a)})
                : std::nullopt,
        }},
    };
  }
};
