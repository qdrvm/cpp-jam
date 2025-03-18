/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <memory>

#include "impl/async_dispatcher_impl.hpp"
#include "subscription.hpp"

namespace jam::se {

  std::shared_ptr<Dispatcher> getDispatcher() {
    return std::make_shared<
        AsyncDispatcher<SubscriptionEngineHandlers::kTotalCount,
                        kThreadPoolSize>>();
  }

}  // namespace jam::se
