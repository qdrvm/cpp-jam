/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "modules/example/example.hpp"

namespace jam::modules {

  ExampleModuleImpl::ExampleModuleImpl(
      std::shared_ptr<jam::modules::ExampleModuleLoader> loader,
      std::shared_ptr<jam::log::LoggingSystem> logsys)
      : loader_(std::move(loader)),
        logsys_(std::move(logsys)),
        logger_(logsys_->getLogger("ExampleModule", "jam")) {}

  void ExampleModuleImpl::on_loaded_success() {
    SL_INFO(logger_, "Loaded success");
  }

  void ExampleModuleImpl::on_loading_is_finished() {
    SL_INFO(logger_, "Loading is finished");
  }

  void ExampleModuleImpl::on_request(std::shared_ptr<const std::string> s) {
    SL_INFO(logger_, "Received request: {}", *s);
  }

  void ExampleModuleImpl::on_response(std::shared_ptr<const std::string> s) {
    SL_INFO(logger_, "Received response: {}", *s);
  }

  void ExampleModuleImpl::on_notify(std::shared_ptr<const std::string> s) {
    SL_INFO(logger_, "Received notification: {}", *s);
  }

}  // namespace jam::modules
