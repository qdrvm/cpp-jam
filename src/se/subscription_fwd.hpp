/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>
#include <memory>

namespace jam {
  enum class SubscriptionEngineHandlers {
    kTest = 0,
    //---------------
    kTotalCount
  };

  static constexpr uint32_t kHandlersCount =
      static_cast<uint32_t>(SubscriptionEngineHandlers::kTotalCount);

  enum class EventTypes {
    // TEST
    kOnTestOperationComplete
  };

  static constexpr uint32_t kThreadPoolSize = 3u;

  namespace se {
    struct Dispatcher;

    template <uint32_t kHandlersCount, uint32_t kPoolSize>
    class SubscriptionManager;

    template <typename EventKey,
              typename Dispatcher,
              typename Receiver,
              typename... Arguments>
    class SubscriberImpl;
  }  // namespace se

  using Dispatcher = se::Dispatcher;
  using Subscription = se::SubscriptionManager<kHandlersCount, kThreadPoolSize>;
  template <typename ObjectType, typename... EventData>
  using BaseSubscriber =
      se::SubscriberImpl<EventTypes, Dispatcher, ObjectType, EventData...>;

}  // namespace jam
