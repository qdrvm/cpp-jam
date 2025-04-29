/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>
#include <iostream>
#include <modules/module_loader.hpp>

namespace jam {
  namespace log {
    class LoggingSystem;
  }
  
  namespace modules {
    class PVMModule;
    
    /**
     * @brief Интерфейс загрузчика для PVMModule
     */
    struct PVMModuleLoader {
      virtual ~PVMModuleLoader() = default;
      
      /**
       * @brief Тестовый метод для вывода приветствия
       */
      virtual void test() = 0;
    };

    /**
     * @brief Интерфейс модуля PVM
     */
    struct PVMModule {
      virtual ~PVMModule() = default;
      
      /**
       * @brief Вызывается при успешной загрузке модуля
       */
      virtual void on_loaded_success() = 0;
      
      /**
       * @brief Тестовый метод для вывода приветствия
       */
      virtual void test() = 0;
    };
  }  // namespace modules
}  // namespace jam 