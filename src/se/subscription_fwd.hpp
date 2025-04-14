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

  enum class EventTypes {
    // -- Modules

    /// All available modules are loaded
    LoadingIsFinished,

    // -- Example

    /// Example module is loaded
    ExampleModuleIsLoaded,
    /// Example module is unloaded
    ExampleModuleIsUnloaded,
    /// Example notification
    ExampleNotification,
    /// Example request
    ExampleRequest,
    /// Example response
    ExampleResponse,

    // -- Networking

    /// Networking module is loaded
    NetworkingIsLoaded,
    /// Networking module is unloaded
    NetworkingIsUnloaded,
    /// Peer connected
    PeerConnected,
    /// Peer disconnected
    PeerDisconnected,
    /// Data of block is requested
    BlockRequest,
    /// Data of block is respond
    BlockResponse,

    // -- Synchronizer

    /// Synchronizer module is loaded
    SynchronizerIsLoaded,
    /// Synchronizer module is unloaded
    SynchronizerIsUnloaded,
    /// Block announce received
    BlockAnnounceReceived,
    /// New block index discovered
    BlockIndexDiscovered,
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
  }  // namespace se

  using Dispatcher = se::IDispatcher;
  using Subscription =
      se::SubscriptionManager<SubscriptionEngineHandlers::kTotalCount,
                              kThreadPoolSize>;
  template <typename ObjectType, typename... EventData>
  using BaseSubscriber =
      se::SubscriberImpl<EventTypes, Dispatcher, ObjectType, EventData...>;

}  // namespace jam
