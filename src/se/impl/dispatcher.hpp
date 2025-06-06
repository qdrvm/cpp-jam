/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>
#include <optional>

#include "scheduler.hpp"

namespace jam::se {

  struct IDispatcher {
    using Tid = uint32_t;
    using Task = IScheduler::Task;
    using Predicate = IScheduler::Predicate;

    static constexpr Tid kExecuteInPool = std::numeric_limits<Tid>::max();

    virtual ~IDispatcher() = default;

    virtual std::optional<Tid> bind(std::shared_ptr<IScheduler> scheduler) = 0;
    virtual bool unbind(Tid tid) = 0;

    virtual void dispose() = 0;
    virtual void add(Tid tid, Task &&task) = 0;
    virtual void addDelayed(Tid tid,
                            std::chrono::microseconds timeout,
                            Task &&task) = 0;
    virtual void repeat(Tid tid,
                        std::chrono::microseconds timeout,
                        Task &&task,
                        Predicate &&pred) = 0;
  };

}  // namespace jam::se
