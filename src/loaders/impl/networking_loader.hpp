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
#include "modules/networking/networking.hpp"
#include "modules/shared/networking_types.tmp.hpp"
#include "se/subscription.hpp"

namespace jam::loaders {

  class NetworkingLoader final
      : public std::enable_shared_from_this<NetworkingLoader>,
        public Loader,
        public modules::NetworkingLoader {
    struct __T {};
    std::shared_ptr<log::LoggingSystem> logsys_;

    using InitCompleteSubscriber = BaseSubscriber<__T>;
    std::shared_ptr<InitCompleteSubscriber> on_init_complete_;

    std::shared_ptr<BaseSubscriber<qtils::Empty>> on_loading_finished_;

    std::shared_ptr<
        BaseSubscriber<std::shared_ptr<const messages::BlockRequestMessage>>>
        on_block_request_;

   public:
    NetworkingLoader(injector::NodeInjector &injector,
                     std::shared_ptr<log::LoggingSystem> logsys,
                     std::shared_ptr<modules::Module> module)
        : Loader(injector, std::move(module)), logsys_(std::move(logsys)) {}

    NetworkingLoader(const NetworkingLoader &) = delete;
    NetworkingLoader &operator=(const NetworkingLoader &) = delete;

    ~NetworkingLoader() override = default;

    void start() override {
      auto module_accessor = module_->getFunctionFromLibrary<
          std::weak_ptr<jam::modules::Networking>,
          std::shared_ptr<modules::NetworkingLoader>,
          std::shared_ptr<log::LoggingSystem>>("query_module_instance");

      if (not module_accessor) {
        return;
      }

      auto module_internal = (*module_accessor)(shared_from_this(), logsys_);

      on_init_complete_ = se::SubscriberCreator<__T>::template create<
          EventTypes::NetworkingIsLoaded>(
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

      on_block_request_ = se::SubscriberCreator<
          std::shared_ptr<const messages::BlockRequestMessage>>::
          template create<EventTypes::BlockRequest>(
              SubscriptionEngineHandlers::kTest, [module_internal](auto &msg) {
                if (auto m = module_internal.lock()) {
                  m->on_block_request(msg);
                }
              });


      se::getSubscription()->notify(jam::EventTypes::NetworkingIsLoaded);
    }

    void dispatch_block_announce(
        std::shared_ptr<const messages::BlockAnnounceMessage> msg) override {
      se::getSubscription()->notify(jam::EventTypes::BlockAnnounceReceived,
                                    msg);
    }

    void dispatch_block_response(
        std::shared_ptr<const messages::BlockResponseMessage> msg) override {
      se::getSubscription()->notify(jam::EventTypes::BlockResponse,
                                    std::move(msg));
    }
  };
}  // namespace jam::loaders
