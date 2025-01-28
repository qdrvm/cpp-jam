/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>

#include "impl/common.hpp"
#include "impl/subscriber_impl.hpp"
#include "impl/subscription_manager.hpp"
#include "subscription_fwd.hpp"

namespace jam::se {
  std::shared_ptr<Dispatcher> getDispatcher();
  std::shared_ptr<Subscription> getSubscription();

  template <typename... T>
  constexpr void notifyEngine(std::tuple<T...> &&data) {
    std::apply(
        [](auto &...x) { (..., getSubscription()->notify(x.first, x.second)); },
        data);
  }

  template <typename ObjectType, typename... EventData>
  struct SubscriberCreator {
    template <EventTypes key, typename F, typename... Args>
    static auto create(SubscriptionEngineHandlers tid,
                       F &&callback,
                       Args &&...args) {
      auto subscriber = BaseSubscriber<ObjectType, EventData...>::create(
          getSubscription()->getEngine<EventTypes, EventData...>(),
          std::forward<Args>(args)...);
      subscriber->setCallback(
          [f{std::forward<F>(callback)}](auto /*set_id*/,
                                         auto &object,
                                         auto event_key,
                                         EventData... args) mutable {
            assert(key == event_key);
            std::forward<F>(f)(object, std::move(args)...);
          });
      subscriber->subscribe(0, key, tid);
      return subscriber;
    }
  };
}  // namespace jam::se
