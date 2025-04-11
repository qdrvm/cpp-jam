/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>

namespace morum::log {
  class LoggingSystem;
}  // namespace morum::log

namespace morum::app {
  class Configuration;
  class Application;
}  // namespace morum::app

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

   protected:
    std::shared_ptr<class NodeInjectorImpl> pimpl_;
  };

}  // namespace morum::injector
