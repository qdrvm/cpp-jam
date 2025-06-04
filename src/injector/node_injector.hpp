/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>

#include "se/subscription.hpp"

namespace jam::log {
  class LoggingSystem;
}  // namespace jam::log

namespace jam::app {
  class Configuration;
  class Application;
}  // namespace jam::app

namespace jam::injector {

  /**
   * Dependency injector for a universal node. Provides all major components
   * required by the JamNode application.
   */
  class NodeInjector final {
   public:
    explicit NodeInjector(std::shared_ptr<log::LoggingSystem> logging_system,
                          std::shared_ptr<app::Configuration> configuration);

    std::shared_ptr<app::Application> injectApplication();
    std::shared_ptr<Subscription> getSE();

   protected:
    std::shared_ptr<class NodeInjectorImpl> pimpl_;
  };

}  // namespace jam::injector
