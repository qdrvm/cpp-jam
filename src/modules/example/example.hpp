/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <log/logger.hpp>
#include <modules/example/interfaces.hpp>

namespace jam::modules {

  class ExampleModuleImpl final : public jam::modules::ExampleModule {
    jam::modules::ExampleModuleLoader &loader_;
    qtils::SharedRef<jam::log::LoggingSystem> logsys_;
    jam::log::Logger logger_;

   public:
    ExampleModuleImpl(jam::modules::ExampleModuleLoader &loader,
                      qtils::SharedRef<jam::log::LoggingSystem> logsys);

    void on_loaded_success() override;

    void on_loading_is_finished() override;

    void on_request(std::shared_ptr<const std::string> s) override;

    void on_response(std::shared_ptr<const std::string> s) override;

    void on_notify(std::shared_ptr<const std::string> s) override;
  };


}  // namespace jam::modules
