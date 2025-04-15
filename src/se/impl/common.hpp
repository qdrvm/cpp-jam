/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <shared_mutex>

namespace jam::se::utils {

  template <typename To, typename From>
  inline std::shared_ptr<To> reinterpret_pointer_cast(
      const std::shared_ptr<From> &ptr) noexcept {
    return std::shared_ptr<To>(ptr, reinterpret_cast<To *>(ptr.get()));
  }

  template <typename T>
  inline std::weak_ptr<T> make_weak(const std::shared_ptr<T> &ptr) noexcept {
    return ptr;
  }

  struct NoCopy {
    NoCopy(const NoCopy &) = delete;
    NoCopy &operator=(const NoCopy &) = delete;
    NoCopy() = default;
  };

  struct NoMove {
    NoMove(NoMove &&) = delete;
    NoMove &operator=(NoMove &&) = delete;
    NoMove() = default;
  };

  template <typename T, typename M = std::shared_mutex>
  struct SafeObject {
    using Type = T;

    template <typename... Args>
    SafeObject(Args &&...args) : t_(std::forward<Args>(args)...) {}

    template <typename F>
    inline auto exclusiveAccess(F &&f) {
      std::unique_lock lock(cs_);
      return std::forward<F>(f)(t_);
    }

    template <typename F>
    inline auto try_exclusiveAccess(F &&f) {
      std::unique_lock lock(cs_, std::try_to_lock);
      using ResultType = decltype(std::forward<F>(f)(t_));
      constexpr bool is_void = std::is_void_v<ResultType>;
      using OptionalType = std::conditional_t<is_void,
                                              std::optional<std::monostate>,
                                              std::optional<ResultType>>;

      if (lock.owns_lock()) {
        if constexpr (is_void) {
          std::forward<F>(f)(t_);
          return OptionalType(std::in_place);
        } else {
          return OptionalType(std::forward<F>(f)(t_));
        }
      } else {
        return OptionalType();
      }
    }

    template <typename F>
    inline auto sharedAccess(F &&f) const {
      std::shared_lock lock(cs_);
      return std::forward<F>(f)(t_);
    }

   private:
    T t_;
    mutable M cs_;
  };

  template <typename T, typename M = std::shared_mutex>
  using ReadWriteObject = SafeObject<T, M>;

  class WaitForSingleObject final : NoMove, NoCopy {
    std::condition_variable wait_cv_;
    std::mutex wait_m_;
    bool flag_;

   public:
    WaitForSingleObject() : flag_{true} {}

    bool wait(std::chrono::microseconds wait_timeout) {
      std::unique_lock<std::mutex> _lock(wait_m_);
      return wait_cv_.wait_for(_lock, wait_timeout, [&]() {
        auto prev = !flag_;
        flag_ = true;
        return prev;
      });
    }

    void wait() {
      std::unique_lock<std::mutex> _lock(wait_m_);
      wait_cv_.wait(_lock, [&]() {
        auto prev = !flag_;
        flag_ = true;
        return prev;
      });
    }

    void set() {
      {
        std::unique_lock<std::mutex> _lock(wait_m_);
        flag_ = false;
      }
      wait_cv_.notify_one();
    }
  };
}  // namespace jam::se::utils
