/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>

#include "se/subscription.hpp"

namespace morum::log {
  class LoggingSystem;
}  // namespace morum::log

namespace morum::app {
  class Configuration;
  class Application;
}  // namespace morum::app

namespace morum::loaders {
  class Loader;
}  // namespace morum::loaders

namespace morum::modules {
  class Module;
}  // namespace morum::modules

namespace morum::injector {

  /**
   * Dependency injector for a universal node. Provides all major components
   * required by the JamNode application.
   */
  class NodeInjector final {
   public:
    explicit NodeInjector(std::shared_ptr<log::LoggingSystem> logging_system,
                          std::shared_ptr<app::Configuration> configuration);

    std::shared_ptr<app::Application> injectApplication();
    std::unique_ptr<loaders::Loader> register_loader(
        std::shared_ptr<modules::Module> module);

   protected:
    std::shared_ptr<class NodeInjectorImpl> pimpl_;
  };

}  // namespace morum::injector
