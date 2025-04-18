/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */


#include "modules/synchronizer/synchronizer.hpp"

#include "modules/shared/networking_types.tmp.hpp"
#include "modules/shared/synchronizer_types.tmp.hpp"


namespace jam::modules {

  SynchronizerImpl::SynchronizerImpl(
      qtils::StrictSharedPtr<SynchronizerLoader> loader,
      qtils::StrictSharedPtr<log::LoggingSystem> logging_system)
      : loader_(loader),
        logger_(
            logging_system->getLogger("Synchronizer", "synchronizer_module")) {}

  void SynchronizerImpl::on_loaded_success() {
    SL_INFO(logger_, "Loaded success");
  }

  void SynchronizerImpl::on_block_index_discovered(
      std::shared_ptr<const messages::BlockDiscoveredMessage> msg) {
    SL_INFO(logger_, "Block discovered");
  };

  void SynchronizerImpl::on_block_announce(
      std::shared_ptr<const messages::BlockAnnounceMessage> msg) {
    SL_INFO(logger_, "Block announced");

    // tmp
    static const size_t s = reinterpret_cast<size_t>(this);
    static size_t n = 0;
    auto x = std::make_shared<const messages::BlockRequestMessage>(
        messages::BlockRequestMessage{.ctx = {{s, ++n}}});

    loader_->dispatch_block_request(std::move(x));
  };

  void SynchronizerImpl::on_block_response(
      std::shared_ptr<const messages::BlockResponseMessage> msg) {
    SL_INFO(logger_, "Block response is received");
  }

}  // namespace jam::modules
