/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <iostream>

#include <qtils/empty.hpp>
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
    struct __T {};
    std::shared_ptr<log::LoggingSystem> logsys_;

    using InitCompleteSubscriber = BaseSubscriber<__T>;
    std::shared_ptr<InitCompleteSubscriber> on_init_complete_;

    std::shared_ptr<BaseSubscriber<qtils::Empty>> on_loading_finished_;

    std::shared_ptr<BaseSubscriber<std::shared_ptr<const std::string>>>
        on_request_;

    std::shared_ptr<BaseSubscriber<std::shared_ptr<const std::string>>>
        on_response_;

    std::shared_ptr<BaseSubscriber<std::shared_ptr<const std::string>>>
        on_notification_;

   public:
    ExampleLoader(injector::NodeInjector &injector,
                  std::shared_ptr<log::LoggingSystem> logsys,
                  std::shared_ptr<modules::Module> module)
        : Loader(injector, std::move(module)), logsys_(std::move(logsys)) {}

    ExampleLoader(const ExampleLoader &) = delete;
    ExampleLoader &operator=(const ExampleLoader &) = delete;

    ~ExampleLoader() override = default;

    void start() override {
      auto module_accessor = module_->getFunctionFromLibrary<
          std::weak_ptr<jam::modules::ExampleModule>,
          std::shared_ptr<modules::ExampleModuleLoader>,
          std::shared_ptr<log::LoggingSystem>>("query_module_instance");

      if (not module_accessor) {
        return;
      }

      auto module_internal = (*module_accessor)(shared_from_this(), logsys_);

      on_init_complete_ = se::SubscriberCreator<__T>::template create<
          EventTypes::ExampleModuleIsLoaded>(
          SubscriptionEngineHandlers::kTest, [module_internal](auto &) {
            if (auto m = module_internal.lock()) {
              m->on_loaded_success();
            }
          });

      on_loading_finished_ =
          se::SubscriberCreator<qtils::Empty>::template create<
              EventTypes::LoadingIsFinished>(
              SubscriptionEngineHandlers::kTest, [module_internal](auto &) {
                if (auto m = module_internal.lock()) {
                  m->on_loading_is_finished();
                }
              });

      on_request_ = se::SubscriberCreator<std::shared_ptr<const std::string>>::
          template create<EventTypes::ExampleRequest>(
              SubscriptionEngineHandlers::kTest, [module_internal](auto &msg) {
                if (auto m = module_internal.lock()) {
                  m->on_request(msg);
                }
              });

      on_response_ = se::SubscriberCreator<std::shared_ptr<const std::string>>::
          template create<EventTypes::ExampleResponse>(
              SubscriptionEngineHandlers::kTest, [module_internal](auto &msg) {
                if (auto m = module_internal.lock()) {
                  m->on_response(msg);
                }
              });

      on_notification_ =
          se::SubscriberCreator<std::shared_ptr<const std::string>>::
              template create<EventTypes::ExampleNotification>(
                  SubscriptionEngineHandlers::kTest,
                  [module_internal](auto &msg) {
                    if (auto m = module_internal.lock()) {
                      m->on_notify(msg);
                    }
                  });

      se::getSubscription()->notify(jam::EventTypes::ExampleModuleIsLoaded);
    }

    void dispatch_request(std::shared_ptr<const std::string> s) override {
      se::getSubscription()->notify(jam::EventTypes::ExampleRequest, s);
    }

    void dispatch_response(std::shared_ptr<const std::string> s) override {
      se::getSubscription()->notify(jam::EventTypes::ExampleResponse, s);
    }

    void dispatch_notify(std::shared_ptr<const std::string> s) override {
      se::getSubscription()->notify(jam::EventTypes::ExampleNotification, s);
    }
  };
}  // namespace jam::loaders
