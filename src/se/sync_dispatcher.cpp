/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */


#include "subscription.hpp"

#include "impl/sync_dispatcher_impl.hpp"

namespace jam::se {

  std::shared_ptr<Dispatcher> getDispatcher() {
    return std::make_shared<
        subscription::SyncDispatcher<SubscriptionEngineHandlers::kTotalCount,
                                     kThreadPoolSize>>();
  }

}  // namespace iroha
