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

  struct ExampleModuleLoader {
    virtual ~ExampleModuleLoader() = default;

    virtual void dispatch_request(std::shared_ptr<const std::string>) = 0;
    virtual void dispatch_response(std::shared_ptr<const std::string>) = 0;
    virtual void dispatch_notify(std::shared_ptr<const std::string>) = 0;
  };

  struct ExampleModule {
    virtual ~ExampleModule() = default;
    virtual void on_loaded_success() = 0;
    virtual void on_loading_is_finished() = 0;

    virtual void on_request(std::shared_ptr<const std::string>) = 0;
    virtual void on_response(std::shared_ptr<const std::string>) = 0;
    virtual void on_notify(std::shared_ptr<const std::string>) = 0;
  };

}  // namespace jam::modules

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
