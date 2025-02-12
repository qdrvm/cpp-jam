/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <deque>
#include <stdexcept>
#include <variant>

#include <TODO_qtils/macro/move.hpp>

#include "coro/handler.hpp"
#include "coro/set_thread.hpp"

namespace jam {
  template <typename T>
  class SharedFuture {
   public:
    using Self = std::shared_ptr<SharedFuture<T>>;

    SharedFuture(IoContextPtr io_context_ptr) : MOVE_(io_context_ptr) {}

    static Coro<bool> ready(Self self) {
      SET_CORO_THREAD(self->io_context_ptr_);
      co_return std::holds_alternative<T>(self->state_);
    }

    /**
     * Resumes coroutine immediately or inside `set`.
     */
    static Coro<T> get(Self self) {
      SET_CORO_THREAD(self->io_context_ptr_);
      if (auto *value = std::get_if<T>(&self->state_)) {
        co_return *value;
      }
      auto &handlers = std::get<Handlers>(self->state_);
      co_return co_await coroHandler<T>([&](CoroHandler<T> &&handler) {
        handlers.emplace_back(std::move(handler));
      });
    }

    /**
     * Set value and wake waiting coroutines.
     * Coroutines may complete before `set` returns.
     */
    static Coro<void> set(Self self, T value) {
      SET_CORO_THREAD(self->io_context_ptr_);
      if (std::holds_alternative<T>(self->state_)) {
        throw std::logic_error{"SharedFuture::set must be called once"};
      }
      auto handlers = std::move(std::get<Handlers>(self->state_));
      self->state_ = std::move(value);
      auto &state_value = std::get<T>(self->state_);
      for (auto &handler : handlers) {
        handler(state_value);
      }
    }

   private:
    using Handlers = std::deque<CoroHandler<T>>;

    IoContextPtr io_context_ptr_;
    std::variant<Handlers, T> state_;
  };
}  // namespace jam
