/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jam_types/types.tmp.hpp>
#include <metrics/impl/session_impl.hpp>
#include <modules/module_loader.hpp>
#include <modules/shared/networking_types.tmp.hpp>
#include <modules/shared/synchronizer_types.tmp.hpp>
#include <qtils/create_smart_pointer_macros.hpp>
#include <qtils/strict_sptr.hpp>

namespace jam::modules {

  struct SynchronizerLoader {
    virtual ~SynchronizerLoader() = default;

    virtual void dispatch_block_request(
        std::shared_ptr<const messages::BlockRequestMessage> msg) = 0;
  };

  struct Synchronizer {
    virtual ~Synchronizer() = default;
    virtual void on_loaded_success() = 0;

    /// New block discovered by block announce
    /// Expected from a network subsystem
    virtual void on_block_announce(
        std::shared_ptr<const messages::BlockAnnounceMessage> msg) = 0;

    /// New block discovered (i.e., by peer's state view update)
    virtual void on_block_index_discovered(
        std::shared_ptr<const messages::BlockDiscoveredMessage> msg) = 0;

    /// BlockResponse has received
    virtual void on_block_response(
        std::shared_ptr<const messages::BlockResponseMessage> msg) = 0;
  };

}  // namespace jam::modules

namespace jam::modules {

  class SynchronizerImpl final : public Singleton<Synchronizer>,
                                 public Synchronizer {
   public:
    static std::shared_ptr<Synchronizer> instance;
    CREATE_SHARED_METHOD(SynchronizerImpl);

    SynchronizerImpl(qtils::StrictSharedPtr<SynchronizerLoader> loader,
                     qtils::StrictSharedPtr<log::LoggingSystem> logging_system);

    void on_loaded_success() override;

    void on_block_index_discovered(
        std::shared_ptr<const messages::BlockDiscoveredMessage> msg) override;

    void on_block_announce(
        std::shared_ptr<const messages::BlockAnnounceMessage> msg) override;

    void on_block_response(
        std::shared_ptr<const messages::BlockResponseMessage> msg) override;

   private:
    qtils::StrictSharedPtr<SynchronizerLoader> loader_;
    log::Logger logger_;
  };

}  // namespace jam::modules
