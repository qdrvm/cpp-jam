/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

 #pragma once

 #include "loaders/loader.hpp"
 #include "modules/example/example.hpp"

 
namespace jam::loaders {

    class ExampleLoader final : public Loader {
     public:
        ExampleLoader(injector::NodeInjector injector, modules::Module module) : Loader(std::move(injector), std::move(module)) {

        }

      ExampleLoader(const ExampleLoader &) = delete;
      ExampleLoader &operator=(const ExampleLoader &) = delete;
  
      ~ExampleLoader() = default;
      
      void start() {
        auto function = module_.getFunctionFromLibrary<std::weak_ptr<jam::modules::ExampleModule>(std::shared_ptr<jam::modules::ExampleModuleLoader>, std::shared_ptr<jam::log::LoggingSystem>)>("query_module_instance");
      }
    };
  }  // namespace jam::loaders
  