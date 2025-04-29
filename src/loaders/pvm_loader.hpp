/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "loaders/loader.hpp"
#include "modules/pvm/pvm.hpp"
#include <memory>

namespace jam::loaders {

  /**
   * @brief Загрузчик для модуля PVM
   */
  class PVMLoader : public std::enable_shared_from_this<PVMLoader>, public Loader, public modules::PVMModuleLoader {
   public:
    PVMLoader(injector::NodeInjector &injector,
    std::shared_ptr<log::LoggingSystem> logsys,
             std::shared_ptr<modules::Module> module);

    void start() override;
    
    /**
     * @brief Получить экземпляр модуля PVM
     */
    std::shared_ptr<modules::PVMModule> get_module();
    
    /**
     * @brief Тестовый метод для вывода приветствия
     */
    void test() override;

   private:
    std::weak_ptr<modules::PVMModule> pvm_module_;
    std::shared_ptr<log::LoggingSystem> logsys_;
  };

}  // namespace jam::loaders 