/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <TODO_qtils/macro/make_shared.hpp>
#include <jam/coro/future.hpp>
#include <jam/coro/spawn.hpp>

namespace jam {
  /**
   * Async init flag.
   *   struct Foo {
   *     CoroOutcome<void> init() {
   *       auto init = init_.init(); // dtor will fail incomplete init
   *       ...
   *       init.ready(); // init completed
   *     }
   *     CoroOutcome<void> foo() {
   *       if (not co_await init_.ready()) // init failed
   *       ... // ready
   *     }
   *     CoroInit init_;
   *   }
   */
  class CoroInit {
    class Init {
     public:
      Init(CoroInit &self) : self_{self} {}
      ~Init() {
        self_.set(false);
      }
      void ready() {
        self_.set(true);
      }

     private:
      CoroInit &self_;
    };

   public:
    CoroInit(IoContextPtr io_context_ptr)
        : MOVE_(io_context_ptr), MAKE_SHARED_(future_, io_context_ptr_) {}

    auto init() {
      if (init_called_) {
        throw std::logic_error{"Coro::init init must be called once"};
      }
      init_called_ = true;
      return Init{*this};
    }

    Coro<bool> ready() {
      return future_->get(future_);
    }

   private:
    void set(bool ready) {
      coroSpawn(*io_context_ptr_, [future{future_}, ready]() -> Coro<void> {
        if (not ready and co_await future->ready(future)) {
          co_return;
        }
        co_await future->set(future, ready);
      });
    }

    IoContextPtr io_context_ptr_;
    std::shared_ptr<SharedFuture<bool>> future_;
    bool init_called_ = false;
  };
}  // namespace jam
