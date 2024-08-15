/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <blake2.h>
#include <jam_crust.h>
#include <algorithm>
#include <boost/endian/conversion.hpp>

#include "types.hpp"

struct Blake {
  using Hash = qtils::BytesN<32>;
  blake2b_state state;
  Blake() {
    blake2b_init(&state, sizeof(Hash));
  }
  Blake &update(qtils::BytesIn input) {
    blake2b_update(&state, input.data(), input.size());
    return *this;
  }
  Hash hash() const {
    Hash hash;
    auto state2 = state;
    blake2b_final(&state2, hash.data(), sizeof(Hash));
    return hash;
  }
  static Hash hash(qtils::BytesIn input) {
    return Blake{}.update(input).hash();
  }
};

namespace jam::safrole {
  using types = test_vectors_safrole::tiny;
  using GammaA = decltype(types::State::gamma_a);
  using GammaS = decltype(types::State::gamma_s);
  using GammaZ = decltype(types::State::gamma_z);
  using BandersnatchSignature = decltype(types::TicketEnvelope::signature);
  using BandersnatchKeys = decltype(types::EpochMark::validators);

  inline auto *ring_ctx() {
    static auto ctx = jam_ring_new(types::validators_count);
    return ctx;
  }

  struct TicketBodyLess {
    bool operator()(
        const types::TicketBody &l, const types::TicketBody &r) const {
      return l.id < r.id;
    }
  };

  // [JAM:1.4.5]
  // clang-format off
  const qtils::BytesN<15> X_T = {'j','a','m','_','t','i','c','k','e','t','_','s','e','a','l'};
  // clang-format on

  // [JAM:1.4.4]
  constexpr auto E = types::epoch_length;

  // [JAM:1.4.4]
  constexpr uint32_t N = 2;
  // [JAM:1.4.4]
  constexpr uint32_t K = 16;

  // [JAM:1.4.4]
  constexpr uint32_t Y = E * 5 / 6;

  template <size_t N>
  qtils::BytesN<N> first_bytes(qtils::BytesIn bytes) {
    if (bytes.size() < N) {
      throw std::logic_error("de");
    }
    qtils::BytesN<N> first;
    memcpy(first.data(), bytes.data(), N);
    return first;
  }

  BandersnatchKeys bandersnatch_keys(const types::ValidatorsData &validators) {
    BandersnatchKeys keys;
    for (size_t i = 0; i < keys.size(); ++i) {
      keys[i] = validators[i].bandersnatch;
    }
    return keys;
  }

  // [JAM:3.7.2]
  template <size_t X, size_t Y>
  qtils::BytesN<X + Y> frown(
      const qtils::BytesN<X> &x, const qtils::BytesN<Y> &y) {
    qtils::BytesN<X + Y> xy;
    memcpy(xy.data(), x.data(), X);
    memcpy(xy.data() + X, y.data(), Y);
    return xy;
  }

  template <size_t X>
  qtils::BytesN<X + 1> doubleplus(const qtils::BytesN<X> &x, uint8_t i) {
    return frown(x, std::array{i});
  }

  // [JAM:273]
  template <size_t N>
  qtils::BytesN<N> mathcal_E(uint64_t x) {
    qtils::BytesN<N> out;
    boost::endian::endian_store<uint64_t, N, boost::endian::order::little>(
        out.data(), x);
    return out;
  }
  // [JAM:273]
  template <size_t N>
  auto de(qtils::BytesN<N> x) {
    return boost::endian::
        endian_load<uint64_t, N, boost::endian::order::little>(x.data());
  }

  // [JAM:3.8.1]
  inline types::OpaqueHash mathcal_H(qtils::BytesIn m) {
    return Blake::hash(m);
  }

  inline GammaZ mathcal_O(const BandersnatchKeys &pks) {
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

  // [JAM:310]
  // [JAM:313]
  std::optional<types::OpaqueHash> banderout(
      const BandersnatchSignature &signature) {
    types::OpaqueHash out;
    if (not jam_vrf_output(signature.data(), out.data())) {
      return std::nullopt;
    }
    return out;
  }

  // [JAM:310]
  // [JAM:313]
  inline std::optional<types::OpaqueHash> bandersnatch(const GammaZ &gamma_z,
      qtils::BytesIn input,
      const BandersnatchSignature &signature) {
    types::OpaqueHash out;
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

  auto &circlearrowleft(const auto &s, auto i) {
    return s[i % s.size()];
  }

  struct Epoch {
    uint32_t epoch, phase;
    Epoch(uint32_t slot) : epoch{slot / E}, phase{slot % E} {}
  };

  inline std::pair<types::State, types::Output> transition(
      const types::State &state, const types::Input &input) {
    const auto &[H_t, banderout_H_v, E_T] = input;
    const auto
        &[tau, eta, lambda, kappa, gamma_k, iota, gamma_a, gamma_s, gamma_z] =
            state;
    const auto &[eta_0, eta_1, eta_2, eta_3] = eta;
    using Error = types::CustomErrorCode;
    const auto error = [&](Error error) {
      return std::make_pair(state, types::Output{error});
    };

    if (H_t <= state.tau) {
      return error(Error::bad_slot);
    }

    // [JAM:46]
    const auto tau_tick = H_t;

    // [JAM:47]
    const Epoch epoch{state.tau}, epoch_tick{tau_tick};
    const auto &[e, m] = epoch;
    const auto &[e_tick, m_tick] = epoch_tick;
    const auto change_epoch = e_tick != e;

    // [JAM:75]
    if (E_T.size() > K) {
      throw std::logic_error("not covered by test vectors");
    }
    if (m_tick >= Y and not E_T.empty()) {
      return std::make_pair(state, types::Output{Error::unexpected_ticket});
    }

    // [JAM:59]
    // TODO: offenders
    const auto phi = [](const types::ValidatorsData &k) { return k; };
    // [JAM:58]
    const auto [gamma_tick_k, kappa_tick, lambda_tick, gamma_tick_z] = change_epoch ?
      [&](const types::ValidatorsData&gamma_tick_k) {
        return std::tuple{gamma_tick_k, gamma_k, kappa, mathcal_O(bandersnatch_keys(gamma_tick_k))};
      }(phi(iota)) :
      std::tuple{gamma_k,kappa,lambda,gamma_z};

    // [JAM:67]
    const auto eta_tick_0 = mathcal_H(frown(eta_0, banderout_H_v));

    // [JAM:68]
    const auto [eta_tick_1, eta_tick_2, eta_tick_3] = change_epoch
        ? std::tuple{eta_0, eta_1, eta_2}
        : std::tuple{eta_1, eta_2, eta_3};

    std::vector<types::TicketBody> n;
    for (auto &[r, p] : E_T) {
      // [JAM:74]
      if (r >= N) {
        return error(Error::bad_ticket_attempt);
      }
      const auto m = frown(X_T, doubleplus(eta_tick_2, r));
      const auto y = bandersnatch(gamma_z, m, p);
      if (not y) {
        return error(Error::bad_ticket_proof);
      }
      // [JAM:76]
      n.emplace_back(types::TicketBody{*y, r});
    }
    // [JAM:77]
    if (not std::is_sorted(n.begin(), n.end(), TicketBodyLess{})) {
      return error(Error::bad_ticket_order);
    }

    // [JAM:79]
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
      // [JAM:78]
      if (gamma_tick_a.size() != gamma_a.size() + n.size()) {
        return error(Error::duplicate_ticket);
      }
    }
    if (gamma_tick_a.size() > E) {
      gamma_tick_a.resize(E);
    }

    // [JAM:70]
    const auto Z = [](const GammaA &s) {
      types::TicketsBodies tickets;
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
    // [JAM:71]
    const auto F = [](const types::OpaqueHash &r,
                       const types::ValidatorsData &k) {
      types::EpochKeys keys;
      for (uint32_t i = 0; i < E; ++i) {
        keys[i] = circlearrowleft(
            k, de(first_bytes<4>(mathcal_H(frown(r, mathcal_E<4>(i))))))
                      .bandersnatch;
      }
      return keys;
    };
    // [JAM:69]
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
            .iota = iota,  // TODO
            .gamma_a = gamma_tick_a,
            .gamma_s = gamma_tick_s,
            .gamma_z = gamma_tick_z,
        },
        types::Output{types::OutputMarks{
            // [JAM:72]
            .epoch_mark = change_epoch ? std::make_optional(types::EpochMark{
                              eta_tick_1, bandersnatch_keys(gamma_tick_k)})
                                       : std::nullopt,

            // [JAM:73]
            .tickets_mark =
                e_tick == e && m < Y && Y <= m_tick && gamma_a.size() == E
                ? std::make_optional(types::TicketsMark{Z(gamma_a)})
                : std::nullopt,
        }},
    };
  }
}  // namespace jam::safrole
