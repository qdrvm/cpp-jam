/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */


#include "modules/synchronizer/synchronizer.hpp"


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

  void SynchronizerImpl::on_block_block_announce(
      std::shared_ptr<const messages::BlockAnnounceMessage> msg) {
    SL_INFO(logger_, "Block announced");
  };

}  // namespace jam::modules
