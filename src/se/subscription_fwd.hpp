/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>

namespace jam {
  enum SubscriptionEngineHandlers {
    kTest = 0,
    //---------------
    kTotalCount
  };

  enum EventTypes {
    // TEST
    kOnTestOperationComplete
  };

  static constexpr uint32_t kThreadPoolSize = 3u;

  namespace se {
    struct IDispatcher;

    template <uint32_t kHandlersCount, uint32_t kPoolSize>
    class SubscriptionManager;

    template <typename EventKey,
              typename Dispatcher,
              typename Receiver,
              typename... Arguments>
    class SubscriberImpl;
  }  // namespace jam::se

  using Dispatcher = se::IDispatcher;
  using Subscription =
      se::SubscriptionManager<SubscriptionEngineHandlers::kTotalCount,
                                   kThreadPoolSize>;
  template <typename ObjectType, typename... EventData>
  using BaseSubscriber = se::
      SubscriberImpl<EventTypes, Dispatcher, ObjectType, EventData...>;

}  // namespace jam::se
