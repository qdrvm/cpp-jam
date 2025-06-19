/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <list>
#include <set>

namespace morum::authorizations {
  namespace types = morum::test_vectors;

  auto asSet(auto &&r) {
    return std::set(r.begin(), r.end());
  }

  auto asVec(auto &&r) {
    return std::vector(r.begin(), r.end());
  }

  /// Given state and input, derive next state and output.
  inline std::pair<types::authorizations::State, types::authorizations::Output>
  transition(const types::Config &config,
             const types::authorizations::State &state,
             const types::authorizations::Input &input) {
    // (137)

    // [GP 0.4.5 8 85]
    // [α[c]] - set of authorizers allowable for a particular core
    // state.auth_pools;

    // [GP 0.4.5 8 85]
    // [φ[c]] - the core’s current authorizer queue, from which we draw values
    // to fill the pool
    // The portion of state φ may be altered only through an exogenous call made
    // from the accumulate logic of an appropriately privileged service.
    const auto &queues = state.auth_queues;

    // Since α′ is dependent on φ′, practically speaking, this step must be
    // computed after accumulation, the stage in which φ′ is defined.

    // [α'[c]]
    auto pools = [&] {
      std::vector<std::list<types::AuthorizerHash>> pools{config.cores_count};
      for (types::CoreIndex core = 0; core < config.cores_count; ++core) {
        const auto &pool = state.auth_pools[core];
        assert(pool.size() <= config.auth_pool_max_size);
        pools[core] = {pool.begin(), pool.end()};
      }
      return pools;
    }();

    // The state transition of a block involves placing a new authorization into
    // the pool from the queue

    std::vector deleted(config.cores_count, false);

    // remove authorizer of input
    for (auto &[core, authorizer] : input.auths) {
      auto &pool = pools[core];

      if (erase(pool, authorizer) != 0) {
        deleted[core] = true;

        assert(state.auth_queues[core].size() == config.auth_queue_size);
        const auto index = input.slot % config.auth_queue_size;
        const auto &queueing = state.auth_queues[core][index];
        pool.emplace_back(queueing);
      }
    }

    // Note that we utilize the guarantees extrinsic EG to remove the oldest
    // authorizer which has been used to justify a guaranteed work-package in
    // the current block.
    for (types::CoreIndex core = 0; core < config.cores_count; ++core) {
      if (not deleted[core]) {
        auto &pool = pools[core];
        if (pool.size() == config.auth_pool_max_size) {
          pool.pop_front();
        }
      }
    }

    // draw authorizers to fill the pool
    for (types::CoreIndex core = 0; core < config.cores_count; ++core) {
      auto &pool = pools[core];
      if (pool.size() < config.auth_pool_max_size) {
        assert(state.auth_queues[core].size() == config.auth_queue_size);
        const auto index = input.slot % config.auth_queue_size;
        const auto &queueing = state.auth_queues[core][index];
        pool.emplace_back(queueing);
      }
    }

    types::authorizations::State new_state;

    new_state.auth_pools.resize(config.cores_count);
    for (types::CoreIndex core = 0; core < config.cores_count; ++core) {
      auto &pool = pools[core];
      new_state.auth_pools[core] = {pool.begin(), pool.end()};
    }

    new_state.auth_queues = queues;

    return {new_state, qtils::Empty{}};
  }
}  // namespace morum::authorizations
