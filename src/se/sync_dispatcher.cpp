/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <memory>
#include "subscription.hpp"

#include "impl/sync_dispatcher_impl.hpp"

namespace jam::se {

  std::shared_ptr<Dispatcher> getDispatcher() {
    return std::make_shared<
        SyncDispatcher<SubscriptionEngineHandlers::kTotalCount,
                                     kThreadPoolSize>>();
  }

}  // namespace jam::se
