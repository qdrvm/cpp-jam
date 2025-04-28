/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <log/logger.hpp>
#include <modules/synchronizer/interfaces.hpp>
#include <qtils/create_smart_pointer_macros.hpp>
#include <qtils/strict_sptr.hpp>
#include <utils/ctor_limiters.hpp>

namespace jam::modules {

  class SynchronizerImpl final : public Singleton<Synchronizer>,
                                 public Synchronizer {
   public:
    static std::shared_ptr<Synchronizer> instance;
    CREATE_SHARED_METHOD(SynchronizerImpl);

    SynchronizerImpl(qtils::StrictSharedPtr<SynchronizerLoader> loader,
                     qtils::StrictSharedPtr<log::LoggingSystem> logging_system);

    void on_loaded_success() override;

    // Synchronizer
    void on_block_announcement_handshake(
        std::shared_ptr<const messages::BlockAnnouncementHandshakeReceived> msg)
        override;
    void on_block_announce(
        std::shared_ptr<const messages::BlockAnnouncementReceived> msg)
        override;
    void on_block_index_discovered(
        std::shared_ptr<const messages::BlockDiscoveredMessage> msg) override;
    void on_block_response(
        std::shared_ptr<const messages::BlockResponseReceived> msg) override;

   private:
    qtils::StrictSharedPtr<SynchronizerLoader> loader_;
    log::Logger logger_;

    std::unordered_map<
        RequestId,
        std::function<void(
            std::shared_ptr<const messages::BlockResponseReceived> msg)>>
        block_response_callbacks_;
  };

}  // namespace jam::modules
