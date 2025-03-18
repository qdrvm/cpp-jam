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

    std::optional<const char*> get_module_name_and_version() {
        auto result = module_.getFunctionFromLibrary<const char*()>("get_module_name_and_version");
        if (result) {
            return (*result)();
        }
        return std::nullopt;
    }    

   protected:
    injector::NodeInjector injector_;
    modules::Module module_;
  };
}  // namespace jam::loaders
