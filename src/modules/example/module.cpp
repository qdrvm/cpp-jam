/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <modules/example/example.hpp>

#define MODULE_C_API extern "C" __attribute__((visibility("default")))
#define MODULE_API __attribute__((visibility("default")))

MODULE_C_API const char *loader_id() {
  return "ExampleLoader";
}

MODULE_C_API const char *module_info() {
  return "ExampleModule v1.0";
}

class ExampleModuleImpl final : public jam::modules::ExampleModule {
  qtils::StrictSharedPtr<jam::modules::ExampleModuleLoader> loader_;
  qtils::StrictSharedPtr<jam::log::LoggingSystem> logsys_;
  jam::log::Logger logger_;

 public:
  ExampleModuleImpl(std::shared_ptr<jam::modules::ExampleModuleLoader> loader,
                    std::shared_ptr<jam::log::LoggingSystem> logsys)
      : loader_(std::move(loader)),
        logsys_(std::move(logsys)),
        logger_(logsys_->getLogger("ExampleModule", "jam")) {}

  void on_loaded_success() override {
    SL_INFO(logger_, "Loaded success");
  }

  void on_loading_is_finished() override {
    SL_INFO(logger_, "Loading is finished");
  }

  void on_request(std::shared_ptr<const std::string> s) override {
    SL_INFO(logger_, "Received request: {}", *s);
  }

  void on_response(std::shared_ptr<const std::string> s) override {
    SL_INFO(logger_, "Received response: {}", *s);
  }

  void on_notify(std::shared_ptr<const std::string> s) override {
    SL_INFO(logger_, "Received notification: {}", *s);
  }
};

static std::shared_ptr<ExampleModuleImpl> module_instance;

MODULE_C_API std::weak_ptr<jam::modules::ExampleModule> query_module_instance(
    std::shared_ptr<jam::modules::ExampleModuleLoader> loader,
    std::shared_ptr<jam::log::LoggingSystem> logger) {
  if (!module_instance) {
    module_instance = std::make_shared<ExampleModuleImpl>(std::move(loader),
                                                          std::move(logger));
  }
  return module_instance;
}

MODULE_C_API void release_module_instance() {
  module_instance.reset();
}
