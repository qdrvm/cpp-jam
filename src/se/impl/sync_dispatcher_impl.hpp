/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common.hpp"
#include "dispatcher.hpp"

namespace jam::se {

  template <uint32_t kCount, uint32_t kPoolSize>
  class SyncDispatcher final : public IDispatcher,
                               utils::NoCopy,
                               utils::NoMove {
   private:
    using Parent = IDispatcher;

   public:
    SyncDispatcher() = default;

    void dispose() override {}

    void add(typename Parent::Tid /*tid*/,
             typename Parent::Task &&task) override {
      task();
    }

    void addDelayed(typename Parent::Tid /*tid*/,
                    std::chrono::microseconds /*timeout*/,
                    typename Parent::Task &&task) override {
      task();
    }

    void repeat(Tid tid,
                std::chrono::microseconds timeout,
                typename Parent::Task &&task,
                typename Parent::Predicate &&pred) override {
      if (!pred || pred()) {
        task();
      }
    }

    std::optional<Tid> bind(std::shared_ptr<IScheduler> scheduler) override {
      if (!scheduler) {
        return std::nullopt;
      }

      return kCount;
    }

    bool unbind(Tid tid) override {
      return tid == kCount;
    }
  };

}  // namespace jam::se
