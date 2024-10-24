/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <qtils/option_take.hpp>

#include "../common.hpp"
#include "types.hpp"

namespace jam::history {
  namespace types = test_vectors_history;

  /**
   * The size of recent history, in blocks
   */
  // [GP 0.4.3 I.4.4]
  // https://github.com/gavofyork/graypaper/blob/v0.4.3/text/definitions.tex#L266
  constexpr uint32_t H = 8;

  // [GP 0.4.3 E.2 327]
  // https://github.com/gavofyork/graypaper/blob/v0.4.3/text/merklization.tex#L205
  inline types::Mmr mathcal_A(types::Mmr r, types::Hash l) {
    for (size_t n = 0; n < r.peaks.size(); ++n) {
      if (not r.peaks[n]) {
        r.peaks[n] = l;
        return r;
      }
      auto r_n = qtils::optionTake(r.peaks[n]).value();
      l = mathcal_H_K(frown(r_n, l));
    }
    r.peaks.emplace_back(l);
    return r;
  }

  /**
   * Given state and input, derive next state and output.
   */
  inline std::pair<types::State, types::Output> transition(
      const types::State &state, const types::Input &input) {
    // [GP 0.4.3 7 84]
    // https://github.com/gavofyork/graypaper/blob/v0.4.3/text/recent_history.tex#L32
    std::vector beta_tick(state.beta.size() >= H ? std::next(state.beta.begin())
                                                 : state.beta.begin(),
        state.beta.end());
    // [GP 0.4.3 7 82]
    // https://github.com/gavofyork/graypaper/blob/v0.4.3/text/recent_history.tex#L12
    if (not beta_tick.empty()) {
      beta_tick.back().state_root = input.parent_state_root;
    }
    // [GP 0.4.3 7 83]
    // https://github.com/gavofyork/graypaper/blob/v0.4.3/text/recent_history.tex#L20
    types::Mmr mmr;
    if (not state.beta.empty()) {
      mmr = state.beta.back().mmr;
    }
    auto mmr_tick = mathcal_A(mmr, input.accumulate_root);
    beta_tick.emplace_back(types::BlockInfo{
        .header_hash = input.header_hash,
        .mmr = mmr_tick,
        .state_root = types::Hash{},
        .reported = input.work_packages,
    });
    return std::make_pair(types::State{.beta = beta_tick}, types::Output{});
  }
}  // namespace jam::history
