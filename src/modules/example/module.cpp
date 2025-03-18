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
  std::shared_ptr<jam::modules::ExampleModuleLoader> loader_;
  std::shared_ptr<jam::log::LoggingSystem> logger_;

 public:
  ExampleModuleImpl(std::shared_ptr<jam::modules::ExampleModuleLoader> loader,
                    std::shared_ptr<jam::log::LoggingSystem> logger)
      : loader_(std::move(loader)), logger_(std::move(logger)) {}

  void on_loaded_success() override {
    auto l = logger_->getLogger("ExampleModule", "jam");
    SL_INFO(l, "Loaded success");
  }
};
static std::shared_ptr<ExampleModuleImpl> exmpl_mod;


MODULE_C_API std::weak_ptr<jam::modules::ExampleModule> query_module_instance(
    std::shared_ptr<jam::modules::ExampleModuleLoader> loader,
    std::shared_ptr<jam::log::LoggingSystem> logger) {
  if (!exmpl_mod) {
    exmpl_mod = std::make_shared<ExampleModuleImpl>(std::move(loader),
                                                    std::move(logger));
  }
  return exmpl_mod;
}

MODULE_C_API void release_module_instance() {
  exmpl_mod.reset();
}
