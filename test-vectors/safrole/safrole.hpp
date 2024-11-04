/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jam/bandersnatch.hpp>
#include <test-vectors/common.hpp>
#include <test-vectors/safrole/types.hpp>
#include <unordered_map>

namespace jam::safrole {
  namespace types = test_vectors_safrole;
  using BandersnatchSignature = decltype(types::TicketEnvelope::signature);

  // [GP 0.3.6 G 307]
  // [GP 0.3.6 G 310]
  inline std::optional<types::OpaqueHash> banderout(
      const BandersnatchSignature &signature) {
    return bandersnatch::output(signature);
  }

  struct TicketBodyLess {
    bool operator()(
        const types::TicketBody &l, const types::TicketBody &r) const {
      return l.id < r.id;
    }
  };

  using GammaA = decltype(types::State::gamma_a);
  using GammaZ = decltype(types::State::gamma_z);
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

  // [GP 0.3.6 I.4.5]
  // clang-format off
  constexpr qtils::BytesN<15> X_T = {'j','a','m','_','t','i','c','k','e','t','_','s','e','a','l'};
  // clang-format on

  // [GP 0.3.6 I.4.4]
  constexpr uint32_t N = 2;
  // [GP 0.3.6 I.4.4]
  constexpr uint32_t K = 16;

  inline BandersnatchKeys bandersnatch_keys(
      const types::ValidatorsData &validators) {
    BandersnatchKeys keys;
    for (auto &validator : validators.v) {
      keys.v.emplace_back(validator.bandersnatch);
    }
    return keys;
  }

  // [GP 0.3.6 G 308]
  inline GammaZ mathcal_O(
      const types::Config &config, const BandersnatchKeys &pks) {
    return ring_ctx(config).commitment(pks.v).value();
  }

  // [GP 0.3.6 G 309]
  inline std::optional<types::OpaqueHash> bandersnatch(
      const types::Config &config,
      const GammaZ &gamma_z,
      qtils::BytesIn input,
      const BandersnatchSignature &signature) {
    return ring_ctx(config).verifier(gamma_z).verify(input, signature);
  }

  // [GP 0.3.6 6.1 46]
  struct Epoch {
    uint32_t epoch, phase;
    Epoch(uint32_t E, uint32_t slot) : epoch{slot / E}, phase{slot % E} {}
  };

  /**
   * Given state and input, derive next state and output.
   */
  inline std::pair<types::State, types::Output> transition(
      const types::Config &config,
      const types::State &state,
      const types::Input &input) {
    // [GP 0.3.6 I.4.4]
    const auto E = config.epoch_length;
    // [GP 0.3.6 I.4.4]
    const auto Y = E * 5 / 6;

    const auto &[H_t, banderout_H_v, E_T, offenders_tick] = input;
    const auto
        &[tau, eta, lambda, kappa, gamma_k, iota, gamma_a, gamma_s, gamma_z] =
            state;
    const auto &[eta_0, eta_1, eta_2, eta_3] = eta;
    using Error = types::CustomErrorCode;
    const auto error = [&](Error error) {
      return std::make_pair(state, types::Output{error});
    };

    // [GP 0.3.6 5 41]
    if (H_t <= state.tau) {
      return error(Error::bad_slot);
    }

    // [GP 0.3.6 6.1 45]
    const auto tau_tick = H_t;

    // [GP 0.3.6 6.1 46]
    const Epoch epoch{E, state.tau}, epoch_tick{E, tau_tick};
    const auto &[e, m] = epoch;
    const auto &[e_tick, m_tick] = epoch_tick;
    const auto change_epoch = e_tick != e;

    // [GP 0.3.6 6.7 74]
    if (E_T.size() > K) {
      throw std::logic_error("not covered by test vectors");
    }
    if (m_tick >= Y and not E_T.empty()) {
      return std::make_pair(state, types::Output{Error::unexpected_ticket});
    }

    // [GP 0.3.6 6.3 58]
    const auto phi = [&](const types::ValidatorsData &k) {
      types::ValidatorsData k_tick;
      for (auto &validator : k.v) {
        k_tick.v.emplace_back(
            std::ranges::find(offenders_tick, validator.ed25519)
                    != offenders_tick.end()
                ? types::ValidatorData{}
                : validator);
      }
      return k_tick;
    };
    // [GP 0.3.6 6.3 57]
    const auto [gamma_tick_k, kappa_tick, lambda_tick, gamma_tick_z] = change_epoch ?
      [&](const types::ValidatorsData&gamma_tick_k) {
        return std::tuple{gamma_tick_k, gamma_k, kappa, mathcal_O(config,bandersnatch_keys(gamma_tick_k))};
      }(phi(iota)) :
      std::tuple{gamma_k,kappa,lambda,gamma_z};

    // [GP 0.3.6 6.4 66]
    const auto eta_tick_0 = mathcal_H(frown(eta_0, banderout_H_v));

    // [GP 0.3.6 6.4 67]
    const auto [eta_tick_1, eta_tick_2, eta_tick_3] = change_epoch
        ? std::tuple{eta_0, eta_1, eta_2}
        : std::tuple{eta_1, eta_2, eta_3};

    std::vector<types::TicketBody> n;
    for (auto &[r, p] : E_T) {
      // [GP 0.3.6 6.7 73]
      if (r >= N) {
        return error(Error::bad_ticket_attempt);
      }
      const auto m = frown(X_T, doubleplus(eta_tick_2, r));
      const auto y = bandersnatch(config, gamma_z, m, p);
      if (not y) {
        return error(Error::bad_ticket_proof);
      }
      // [GP 0.3.6 6.7 75]
      n.emplace_back(types::TicketBody{*y, r});
    }
    // [GP 0.3.6 6.7 76]
    if (not std::is_sorted(n.begin(), n.end(), TicketBodyLess{})) {
      return error(Error::bad_ticket_order);
    }

    // [GP 0.3.6 6.7 78]
    std::vector<types::TicketBody> gamma_tick_a;
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
    const auto Z = [&](const GammaA &s) {
      types::TicketsBodies tickets;
      if (s.size() != E) {
        throw std::logic_error{"Z"};
      }
      auto it1 = s.begin(), it2 = std::prev(s.end());
      auto odd = true;
      for (uint32_t i = 0; i < tickets.configSize(config); ++i) {
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
    // [GP 0.3.6 6.5 70]
    const auto F = [&](const types::OpaqueHash &r,
                       const types::ValidatorsData &k) {
      types::EpochKeys keys;
      for (uint32_t i = 0; i < E; ++i) {
        keys.v.emplace_back(circlearrowleft(
            k.v, de(first_bytes<4>(mathcal_H(frown(r, mathcal_E<4>(i))))))
                                .bandersnatch);
      }
      return keys;
    };
    // [GP 0.3.6 6.5 68]
    const auto gamma_tick_s = e_tick == e + 1 && m >= Y && gamma_a.size() == E
        ? types::TicketsOrKeys{Z(gamma_a)}
        : e_tick == e ? gamma_s
                      : types::TicketsOrKeys{F(eta_tick_2, kappa_tick)};

    types::Output output;
    return {
        types::State{
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
        types::Output{types::OutputMarks{
            // [GP 0.3.6 6.6 71]
            .epoch_mark = change_epoch ? std::make_optional(types::EpochMark{
                              eta_tick_1, bandersnatch_keys(gamma_tick_k)})
                                       : std::nullopt,
            // [GP 0.3.6 6.6 72]
            .tickets_mark =
                e_tick == e && m < Y && Y <= m_tick && gamma_a.size() == E
                ? std::make_optional(types::TicketsMark{Z(gamma_a)})
                : std::nullopt,
        }},
    };
  }
}  // namespace jam::safrole
