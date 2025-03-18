/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <chrono>
#include <iostream>

#include <qtils/final_action.hpp>
#include <soralog/impl/configurator_from_yaml.hpp>
#include <soralog/logging_system.hpp>

#include "app/application.hpp"
#include "app/configuration.hpp"
#include "app/configurator.hpp"
#include "injector/node_injector.hpp"
#include "loaders/impl/example_loader.hpp"
#include "log/logger.hpp"
#include "modules/module_loader.hpp"
#include "se/subscription.hpp"

using std::string_view_literals::operator""sv;

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)

namespace {
  void wrong_usage() {
    std::cerr << "Wrong usage.\n"
                 "Run with `--help' argument to print usage\n";
  }

  using jam::app::Application;
  using jam::app::Configuration;
  using jam::injector::NodeInjector;
  using jam::log::LoggingSystem;

  int run_node(std::shared_ptr<LoggingSystem> logsys,
               std::shared_ptr<Configuration> appcfg) {
    auto injector = std::make_unique<NodeInjector>(logsys, appcfg);

    // Load modules
    {
      auto logger = logsys->getLogger("Modules", "jam");
      const std::string path("/home/iceseer/Work/cpp-jam/build/modules/");

      jam::modules::ModuleLoader module_loader(path);
      auto modules = module_loader.get_modules();
      if (modules.has_error()) {
        SL_CRITICAL(logger, "Failed to load modules from path: {}", path);
        return EXIT_FAILURE;
      }

      std::deque<std::shared_ptr<jam::loaders::Loader>> loaders;
      for (const auto &module : modules.value()) {
        if ("ExampleLoader" == module->get_loader_id()) {
          auto loader = std::make_shared<jam::loaders::ExampleLoader>(
              *injector, logsys, module);
          if (auto info = loader->module_info()) {
            SL_INFO(logger, "> Module: {} [{}]", *info, module->get_path());
            loaders.emplace_back(loader);
            loader->start();
          }
        }
      }
    }

    auto logger = logsys->getLogger("Main", jam::log::defaultGroupName);
    auto app = injector->injectApplication();
    SL_INFO(logger, "Node started. Version: {} ", appcfg->nodeVersion());

    app->run();

    SL_INFO(logger, "Node stopped");
    logger->flush();

    return EXIT_SUCCESS;
  }

}  // namespace

int main(int argc, const char **argv, const char **env) {
  qtils::FinalAction dispose_se_on_exit(
      [se_manager{jam::se::getSubscription()}] { se_manager->dispose(); });

  soralog::util::setThreadName("jam-node");

  qtils::FinalAction flush_std_streams_at_exit([] {
    std::cout.flush();
    std::cerr.flush();
  });

  if (argc == 0) {
    // Abnormal run
    wrong_usage();
    return EXIT_FAILURE;
  }

  if (argc == 1) {
    // Run without arguments
    wrong_usage();
    return EXIT_FAILURE;
  }

  auto app_configurator =
      std::make_unique<jam::app::Configurator>(argc, argv, env);

  // Parse CLI args for help, version and config
  if (auto res = app_configurator->step1(); res.has_value()) {
    if (res.value()) {
      return EXIT_SUCCESS;
    }
  } else {
    return EXIT_FAILURE;
  }

  // Setup logging system
  auto logging_system = ({
    auto log_config = app_configurator->getLoggingConfig();
    if (log_config.has_error()) {
      std::cerr << "Logging config is empty.\n";
      return EXIT_FAILURE;
    }

    auto log_configurator = std::make_shared<soralog::ConfiguratorFromYAML>(
        std::shared_ptr<soralog::Configurator>(nullptr), log_config.value());

    auto logging_system =
        std::make_shared<soralog::LoggingSystem>(std::move(log_configurator));

    auto config_result = logging_system->configure();
    if (not config_result.message.empty()) {
      (config_result.has_error ? std::cerr : std::cout)
          << config_result.message << '\n';
    }
    if (config_result.has_error) {
      return EXIT_FAILURE;
    }

    std::make_shared<jam::log::LoggingSystem>(std::move(logging_system));
  });

  // Parse CLI args for help, version and config
  if (auto res = app_configurator->step2(); res.has_value()) {
    if (res.value()) {
      return EXIT_SUCCESS;
    }
  } else {
    return EXIT_FAILURE;
  }

  // Setup config
  auto configuration = ({
    auto logger = logging_system->getLogger("Configurator", "jam");

    auto config_res = app_configurator->calculateConfig(logger);
    if (config_res.has_error()) {
      auto error = config_res.error();
      SL_CRITICAL(logger, "Failed to calculate config: {}", error);
      fmt::println(std::cerr, "Failed to calculate config: {}", error);
      fmt::println(std::cerr, "See more details in the log");
      return EXIT_FAILURE;
    }

    config_res.value();
  });

  int exit_code;

  {
    std::string_view name{argv[1]};

    if (name.substr(0, 1) == "-") {
      // The first argument isn't subcommand, run as node
      exit_code = run_node(logging_system, configuration);
    }

    // else if (false and name == "subcommand-1"s) {
    //   exit_code = execute_subcommend_1(argc - 1, argv + 1);
    // }
    //
    // else if (false and name == "subcommand-2"s) {
    //   exit_code = execute_subcommend_2(argc - 1, argv + 1);
    // }

    else {
      // No subcommand, but argument is not a valid option: begins not with dash
      wrong_usage();
      return EXIT_FAILURE;
    }
  }

  auto logger = logging_system->getLogger("Main", jam::log::defaultGroupName);
  SL_INFO(logger, "All components are stopped");
  logger->flush();

  return exit_code;
}

// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
