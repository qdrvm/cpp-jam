/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <metrics/impl/session_impl.hpp>
#include <modules/module_loader.hpp>
#include <qtils/strict_sptr.hpp>

namespace jam::modules {
  class ExampleModule;
  struct ExampleModuleLoader {
    virtual ~ExampleModuleLoader() = default;
  };

  struct ExampleModule {
    virtual ~ExampleModule() = default;
  };
}

// class BlockTree;

namespace jam::modules {

  // class ExampleModule : public Singleton<ExampleModule> {
  //  public:
  //   static std::shared_ptr<ExampleModule> instance;
  //   CREATE_SHARED_METHOD(ExampleModule);

  //   ExampleModule(qtils::StrictSharedPtr<ExampleModuleLoader> loader,
  //                 qtils::StrictSharedPtr<log::LoggingSystem> logging_system);

  //   qtils::StrictSharedPtr<ExampleModuleLoader> loader_;
  //   log::Logger logger_;
  // };

}  // namespace jam::modules
