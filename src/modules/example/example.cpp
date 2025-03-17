/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "modules/example/example.hpp"

namespace jam::modules {
  std::shared_ptr<ExampleModule> ExampleModule::instance;

  ExampleModule::ExampleModule(
      qtils::StrictSharedPtr<ExampleModuleLoader> loader,
      qtils::StrictSharedPtr<log::LoggingSystem> logging_system)
      : loader_(loader),
        logger_(logging_system->getLogger("ExampleModule", "example_module"))

  {}

}  // namespace jam::modules
