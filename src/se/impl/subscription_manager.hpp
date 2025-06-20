/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <assert.h>
#include <memory>
#include <shared_mutex>
#include <unordered_map>

#include "common.hpp"
#include "compile-time_murmur2.hpp"
#include "dispatcher.hpp"
#include "subscriber.hpp"
#include "subscription_engine.hpp"
#include "utils/ctor_limiters.hpp"

// Cross-platform function name macro for subscription hashing
#ifdef _WIN32
#define FUNCTION_NAME __FUNCSIG__
#else
#define FUNCTION_NAME __PRETTY_FUNCTION__
#endif

// Cross-platform function name macro for subscription hashing
#ifdef _WIN32
#define FUNCTION_NAME __FUNCSIG__
#else
#define FUNCTION_NAME __PRETTY_FUNCTION__
#endif

namespace jam::se {

  /**
   * Class-aggregator that keeps all event engines inside. On notification it
   * selects the appropriate engine and calls notification in it.
   * @tparam kHandlersCount number of supported thread handlers
   * @tparam kPoolSize number of threads in thread pool
   */
  template <uint32_t kHandlersCount, uint32_t kPoolSize>
  class SubscriptionManager final
      : public std::enable_shared_from_this<
            SubscriptionManager<kHandlersCount, kPoolSize>>,
        NonCopyable,
        NonMovable {
   public:
    using Dispatcher = jam::se::Dispatcher;

   private:
    using EngineHash = uint64_t;
    using DispatcherPtr = std::shared_ptr<Dispatcher>;
    using EnginesList = std::unordered_map<EngineHash, std::shared_ptr<void>>;

   private:
    /// Thread handlers dispatcher
    DispatcherPtr dispatcher_;
    std::shared_mutex engines_cs_;
    /// Engines container
    EnginesList engines_;
    std::atomic_flag disposed_;

   private:
    template <typename... Args>
    static constexpr EngineHash getSubscriptionHash() {
      constexpr EngineHash value = CT_MURMUR2(FUNCTION_NAME);
      return value;
    }

   public:
    SubscriptionManager(DispatcherPtr dispatcher)
        : dispatcher_(std::move(dispatcher)) {
      disposed_.clear();
    }

    /**
     * Detaches the dispatcher from all engines and stops thread handlers
     * execution.
     */
    void dispose() {
      if (!disposed_.test_and_set()) {
        {
          std::shared_lock lock(engines_cs_);
          for (auto &descriptor : engines_) {
            std::reinterpret_pointer_cast<IDisposable>(descriptor.second)
                ->dispose();
          }
        }
        dispatcher_->dispose();
      }
    }

    /**
     * Method returns the engine corresponding to current arguments set
     * transmission.
     * @tparam EventKey typeof event enum
     * @tparam Args arguments list of transmitted event data types
     * @return engine object
     */
    template <typename EventKey, typename... Args>
    auto getEngine() {
      using EngineType =
          SubscriptionEngine<EventKey,
                             Dispatcher,
                             Subscriber<EventKey, Dispatcher, Args...>>;
      constexpr auto engineId = getSubscriptionHash<Args...>();
      {
        std::shared_lock lock(engines_cs_);
        if (auto it = engines_.find(engineId); it != engines_.end()) {
          return std::reinterpret_pointer_cast<EngineType>(it->second);
        }
      }
      std::unique_lock lock(engines_cs_);
      if (auto it = engines_.find(engineId); it != engines_.end()) {
        return std::reinterpret_pointer_cast<EngineType>(it->second);
      }

      /// To be sure IDisposable is the first base class, because of later cast
      static_assert(std::is_base_of_v<IDisposable, EngineType>,
                    "Engine type must be derived from IDisposable.");
      assert(uintptr_t(reinterpret_cast<EngineType *>(0x1))
             == uintptr_t(static_cast<IDisposable *>(
                 reinterpret_cast<EngineType *>(0x1))));

      auto obj = std::make_shared<EngineType>(dispatcher_);
      engines_[engineId] = std::reinterpret_pointer_cast<void>(obj);
      return obj;
    }

    /**
     * Make event notification to subscribers that are listening to this event
     * @tparam EventKey typeof event enum
     * @tparam Args arguments list of transmitted event data types
     * @param key event key
     * @param args transmitted data
     */
    template <typename EventKey, typename... Args>
    void notify(const EventKey &key, const Args &...args) {
      notifyDelayed(std::chrono::microseconds(0ull), key, args...);
    }

    /**
     * Make event notification to subscribers that are listening this event
     * after a delay
     * @tparam EventKey typeof event enum
     * @tparam Args arguments list of transmitted event data types
     * @param timeout delay before subscribers will be notified
     * @param key event key
     * @param args transmitted data
     */
    template <typename EventKey, typename... Args>
    void notifyDelayed(std::chrono::microseconds timeout,
                       const EventKey &key,
                       const Args &...args) {
      using EngineType =
          SubscriptionEngine<EventKey,
                             Dispatcher,
                             Subscriber<EventKey, Dispatcher, Args...>>;
      constexpr auto engineId = getSubscriptionHash<Args...>();
      std::shared_ptr<EngineType> engine;
      {
        std::shared_lock lock(engines_cs_);
        if (auto it = engines_.find(engineId); it != engines_.end()) {
          engine = std::reinterpret_pointer_cast<EngineType>(it->second);
        } else {
          return;
        }
      }
      assert(engine);
      engine->notifyDelayed(timeout, key, args...);
    }

    /**
     * Getter to retrieve a dispatcher.
     * @return dispatcher object
     */
    DispatcherPtr dispatcher() const {
      return dispatcher_;
    }
  };
}  // namespace jam::se
