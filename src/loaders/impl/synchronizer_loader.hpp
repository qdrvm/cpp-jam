/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <soralog/logging_system.hpp>

#include "loaders/loader.hpp"
#include "log/logger.hpp"
#include "modules/synchronizer/synchronizer.hpp"
#include "se/subscription.hpp"

namespace jam::loaders {

  class SynchronizerLoader final
      : public std::enable_shared_from_this<SynchronizerLoader>,
        public Loader,
        public modules::SynchronizerLoader {
    std::shared_ptr<log::LoggingSystem> logsys_;
    log::Logger logger_;

    using InitCompleteSubscriber = BaseSubscriber<qtils::Empty>;
    std::shared_ptr<InitCompleteSubscriber> on_init_complete_;

    std::shared_ptr<void> on_block_announcement_handshake_;
    std::shared_ptr<BaseSubscriber<
        qtils::Empty,
        std::shared_ptr<const messages::BlockAnnouncementReceived>>>
        on_block_announce_;

    std::shared_ptr<
        BaseSubscriber<qtils::Empty,
                       std::shared_ptr<const messages::BlockResponseReceived>>>
        on_block_response_;

   public:
    SynchronizerLoader(injector::NodeInjector &injector,
                       std::shared_ptr<log::LoggingSystem> logsys,
                       std::shared_ptr<modules::Module> module)
        : Loader(injector, std::move(module)),
          logsys_(std::move(logsys)),
          logger_(logsys_->getLogger("Synchronizer", "synchronizer_module")) {}

    SynchronizerLoader(const SynchronizerLoader &) = delete;
    SynchronizerLoader &operator=(const SynchronizerLoader &) = delete;

    ~SynchronizerLoader() override = default;

    void start() override {
      auto module_accessor = module_->getFunctionFromLibrary<
          std::weak_ptr<jam::modules::Synchronizer>,
          std::shared_ptr<modules::SynchronizerLoader>,
          std::shared_ptr<log::LoggingSystem>>("query_module_instance");

      if (not module_accessor) {
        return;
      }

      auto module_internal = (*module_accessor)(shared_from_this(), logsys_);

      on_init_complete_ = se::SubscriberCreator<qtils::Empty>::template create<
          EventTypes::SynchronizerIsLoaded>(
          SubscriptionEngineHandlers::kTest, [module_internal, this](auto &) {
            if (auto m = module_internal.lock()) {
              SL_TRACE(logger_, "Handle SynchronizerIsLoaded");
              m->on_loaded_success();
            }
          });

      on_block_announcement_handshake_ = se::SubscriberCreator<
          qtils::Empty,
          std::shared_ptr<const messages::BlockAnnouncementHandshakeReceived>>::
          template create<EventTypes::BlockAnnouncementHandshakeReceived>(
              SubscriptionEngineHandlers::kTest,
              [module_internal, this](auto &, const auto &msg) {
                if (auto m = module_internal.lock()) {
                  SL_TRACE(logger_,
                           "Handle BlockAnnouncementHandshakeReceived");
                  m->on_block_announcement_handshake(msg);
                }
              });

      on_block_announce_ = se::SubscriberCreator<
          qtils::Empty,
          std::shared_ptr<const messages::BlockAnnouncementReceived>>::
          template create<EventTypes::BlockAnnounceReceived>(
              SubscriptionEngineHandlers::kTest,
              [module_internal, this](auto &, const auto &msg) {
                if (auto m = module_internal.lock()) {
                  SL_TRACE(logger_, "Handle BlockAnnounceReceived");
                  m->on_block_announce(msg);
                }
              });

      on_block_response_ = se::SubscriberCreator<
          qtils::Empty,
          std::shared_ptr<const messages::BlockResponseReceived>>::
          template create<EventTypes::BlockResponse>(
              SubscriptionEngineHandlers::kTest,
              [module_internal, this](auto &, const auto &msg) {
                if (auto m = module_internal.lock()) {
                  SL_TRACE(logger_,
                           "Handle BlockResponse; rid={}",
                           msg->for_ctx.rid);
                  m->on_block_response(msg);
                }
              });

      se::getSubscription()->notify(jam::EventTypes::SynchronizerIsLoaded);
    }

    void dispatch_block_request(
        std::shared_ptr<const messages::SendBlockRequest> msg) override {
      SL_TRACE(logger_, "Dispatch BlockRequest; rid={}", msg->ctx.rid);
      se::getSubscription()->notify(jam::EventTypes::BlockRequest, msg);
    }
  };

}  // namespace jam::loaders
