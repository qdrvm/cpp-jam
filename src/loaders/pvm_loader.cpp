/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "loaders/pvm_loader.hpp"
#include <soralog/logging_system.hpp>
#include "log/logger.hpp"
#include <soralog/macro.hpp>
#include <qtils/outcome.hpp>
#include <iostream>

namespace jam::loaders {

  PVMLoader::PVMLoader(injector::NodeInjector &injector,
                       std::shared_ptr<log::LoggingSystem> logsys,
                     std::shared_ptr<modules::Module> module)
      : Loader(injector, std::move(module)), logsys_(std::move(logsys)) {}

  void PVMLoader::start() {
    auto logger = logsys_->getLogger("PVMLoader", "jam");
    
    // Получаем функцию из модуля
    auto query_instance =
        module_->getFunctionFromLibrary<
        std::weak_ptr<modules::PVMModule>, 
        std::shared_ptr<modules::PVMModuleLoader>,
        std::shared_ptr<log::LoggingSystem>>("query_module_instance");
    
    if (query_instance) {
      // Вызываем функцию для получения модуля
      pvm_module_ = (*query_instance)(
          shared_from_this(),
          logsys_);
      
      // Вызываем метод успешной загрузки
      if (auto module = pvm_module_.lock()) {
        module->on_loaded_success();
      }
    } else {
      SL_CRITICAL(logger, "Не удалось найти функцию query_module_instance в модуле");
    }
  }

  std::shared_ptr<modules::PVMModule> PVMLoader::get_module() {
    return pvm_module_.lock();
  }
  
  void PVMLoader::test() {
    std::cout << "Привет из PVMLoader! Тестовое сообщение." << std::endl;
  }

}  // namespace jam::loaders 