/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <iostream>

#include <soralog/logging_system.hpp>

#include "loaders/loader.hpp"
#include "log/logger.hpp"
#include "modules/example/example.hpp"
#include "se/subscription.hpp"

namespace jam::loaders {

  class ExampleLoader final
      : public std::enable_shared_from_this<ExampleLoader>,
        public Loader,
        public modules::ExampleModuleLoader {
    struct __T{};
    std::shared_ptr<log::LoggingSystem> logsys_;

    using InitCompleteSubscriber = BaseSubscriber<__T>;
    std::shared_ptr<InitCompleteSubscriber> on_init_complete_;

   public:
    ExampleLoader(injector::NodeInjector &injector,
                  std::shared_ptr<log::LoggingSystem> logsys,
                  std::shared_ptr<modules::Module> module)
        : Loader(injector, std::move(module)), logsys_(std::move(logsys)) {}

    ExampleLoader(const ExampleLoader &) = delete;
    ExampleLoader &operator=(const ExampleLoader &) = delete;

    ~ExampleLoader() = default;

    void start() {
      auto function = module_->getFunctionFromLibrary<
          std::weak_ptr<jam::modules::ExampleModule>,
          std::shared_ptr<modules::ExampleModuleLoader>,
          std::shared_ptr<log::LoggingSystem>>("query_module_instance");

      if (function) {
        auto module_internal = (*function)(shared_from_this(), logsys_);
        on_init_complete_ = se::SubscriberCreator<__T>::template create<
            EventTypes::kOnTestOperationComplete>(
            SubscriptionEngineHandlers::kTest, [module_internal](auto &) {
              if (auto m = module_internal.lock()) {
                m->on_loaded_success();
              }
            });

        se::getSubscription()->notify(
            jam::EventTypes::kOnTestOperationComplete);
      }
    }
  };
}  // namespace jam::loaders
