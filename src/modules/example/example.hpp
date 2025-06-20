/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <log/logger.hpp>
#include <modules/example/interfaces.hpp>

namespace morum::modules {

  class ExampleModuleImpl final : public morum::modules::ExampleModule {
    [[maybe_unused]] morum::modules::ExampleModuleLoader &loader_;
    qtils::SharedRef<morum::log::LoggingSystem> logsys_;
    morum::log::Logger logger_;

   public:
    ExampleModuleImpl(morum::modules::ExampleModuleLoader &loader,
                      qtils::SharedRef<morum::log::LoggingSystem> logsys);

    void on_loaded_success() override;

    void on_loading_is_finished() override;

    void on_request(std::shared_ptr<const std::string> s) override;

    void on_response(std::shared_ptr<const std::string> s) override;

    void on_notify(std::shared_ptr<const std::string> s) override;
  };


}  // namespace morum::modules
