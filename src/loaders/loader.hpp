/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "injector/node_injector.hpp"
#include "modules/module.hpp"

namespace jam::loaders {

  class Loader {
   public:
    Loader(const Loader &) = delete;
    Loader &operator=(const Loader &) = delete;

    virtual ~Loader() = default;
    virtual void start() = 0;

    std::optional<const char *> module_info() {
      auto result =
          module_->getFunctionFromLibrary<const char *>("module_info");
      if (result) {
        return (*result)();
      }
      return std::nullopt;
    }

    Loader(injector::NodeInjector &injector,
           std::shared_ptr<modules::Module> module)
        : injector_(injector), module_(std::move(module)) {}

   protected:
    injector::NodeInjector &injector_;
    std::shared_ptr<modules::Module> module_;
  };
}  // namespace jam::loaders
