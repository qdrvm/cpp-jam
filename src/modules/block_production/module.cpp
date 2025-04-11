/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <modules/block_production/block_tree.hpp>

#define MODULE_C_API extern "C" __attribute__((visibility("default")))
#define MODULE_API __attribute__((visibility("default")))

MODULE_C_API const char *loader_id() {
  return "BlockTreeLoader";
}

MODULE_C_API const char *module_info() {
  return "BlockTree v0.0";
}

class BlockTreeImpl final : public jam::modules::BlockTree {
  qtils::StrictSharedPtr<jam::modules::BlockTreeLoader> loader_;
  qtils::StrictSharedPtr<jam::log::LoggingSystem> logsys_;
  jam::log::Logger logger_;

 public:
  BlockTreeImpl(std::shared_ptr<jam::modules::BlockTreeLoader> loader,
                std::shared_ptr<jam::log::LoggingSystem> logsys)
      : loader_(std::move(loader)),
        logsys_(std::move(logsys)),
        logger_(logsys_->getLogger("BlockTree", "jam")) {}

  void on_loaded_success() override {
    SL_INFO(logger_, "Loaded success");
  }

  // void do_notify() override {
  //   SL_INFO(logger_, "Doing notify");
  // }

  // void do_request() override {
  //   SL_INFO(logger_, "Doing request");
  // }

  void on_request() override {
    SL_INFO(logger_, "Handle request");
  }

  void on_notify() override {
    SL_INFO(logger_, "Handle notify");
  }
};

static std::shared_ptr<BlockTreeImpl> module_instance;

MODULE_C_API std::weak_ptr<jam::modules::BlockTree> query_module_instance(
    std::shared_ptr<jam::modules::BlockTreeLoader> loader,
    std::shared_ptr<jam::log::LoggingSystem> logger) {
  if (!module_instance) {
    module_instance =
        std::make_shared<BlockTreeImpl>(std::move(loader), std::move(logger));
  }
  return module_instance;
}

MODULE_C_API void release_module_instance() {
  module_instance.reset();
}
