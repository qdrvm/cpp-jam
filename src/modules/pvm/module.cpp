/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "modules/pvm/pvm.hpp"
#include <soralog/logging_system.hpp>
#include "log/logger.hpp"
#include <soralog/macro.hpp>
#include <qtils/outcome.hpp>
#include <system_error>
#include <iostream>

#define MODULE_C_API extern "C" __attribute__((visibility("default")))
#define MODULE_API __attribute__((visibility("default")))

MODULE_C_API const char *loader_id() {
  return "PVMLoader";
}

MODULE_C_API const char *module_info() {
  return "PVMModule v1.0";
}

/**
 * @brief Имплементация модуля PVM
 */
class PVMModuleImpl final : public jam::modules::PVMModule {
  std::shared_ptr<jam::modules::PVMModuleLoader> loader_;
  std::shared_ptr<jam::log::LoggingSystem> logger_;

 public:
  PVMModuleImpl(std::shared_ptr<jam::modules::PVMModuleLoader> loader,
                std::shared_ptr<jam::log::LoggingSystem> logger)
      : loader_(std::move(loader)), logger_(std::move(logger)) {}

  void on_loaded_success() override {
    auto l = logger_->getLogger("PVMModule", "jam");
    if (l) {
      SL_INFO(l, "PVM модуль успешно загружен");
    }
  }
  
  void test() override {
    std::cout << "Привет из PVMModule! Тестовое сообщение." << std::endl;
    
    // Также вызываем тестовый метод загрузчика
    if (loader_) {
      loader_->test();
    }
  }
};

// Единственный экземпляр модуля
static std::shared_ptr<PVMModuleImpl> pvm_mod;

MODULE_C_API std::weak_ptr<jam::modules::PVMModule> query_module_instance(
    std::shared_ptr<jam::modules::PVMModuleLoader> loader,
    std::shared_ptr<jam::log::LoggingSystem> logger) {
  if (!pvm_mod) {
    pvm_mod = std::make_shared<PVMModuleImpl>(std::move(loader),
                                              std::move(logger));
  }
  return pvm_mod;
}

MODULE_C_API void release_module_instance() {
  pvm_mod.reset();
} 