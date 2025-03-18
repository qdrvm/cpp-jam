/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <assert.h>
#include <thread>

#include "scheduler_impl.hpp"

namespace jam::se {

  class ThreadHandler final : public SchedulerBase {
   private:
    std::thread worker_;

   public:
    ThreadHandler() {
      worker_ = std::thread(
          [](ThreadHandler *__this) { return __this->process(); }, this);
    }

    void dispose(bool wait_for_release = true) {
      SchedulerBase::dispose(wait_for_release);
      if (wait_for_release) {
        worker_.join();
      } else {
        worker_.detach();
      }
    }
  };

}  // namespace jam::se
