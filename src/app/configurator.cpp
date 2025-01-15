/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "app/configuration.hpp"

#include <filesystem>
#include <iostream>

#include <boost/program_options.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <qtils/outcome.hpp>

#include "app/build_version.hpp"
#include "app/configurator.hpp"

OUTCOME_CPP_DEFINE_CATEGORY(jam::app, Configurator::Error, e) {
  using E = jam::app::Configurator::Error;
  switch (e) {
    case E::CliArgsParseFailed:
      return "CLI Arguments parse failed";
    case E::ConfigFileParseFailed:
      return "Config file parse failed";
  }
  BOOST_UNREACHABLE_RETURN("Unknown log::Error");
}
namespace jam::app {

  Configurator::Configurator(int argc, const char **argv, const char **env)
      : argc_(argc), argv_(argv), env_(env) {
    config_ = std::make_shared<Configuration>();
  }

  outcome::result<YAML::Node> Configurator::getLoggingConfig() {
    auto logging = (*config_file_)["logging"];
    if (logging.IsDefined()) {
      return logging;
    }
    return YAML::Node{}; // TODO return default logging config
  }

  outcome::result<std::shared_ptr<Configuration>> Configurator::calculateConfig(
      std::shared_ptr<soralog::Logger> logger) {
    return config_;
  }

  outcome::result<bool> Configurator::step1() {
    namespace po = boost::program_options;
    namespace fs = std::filesystem;

    // clang-format off
    po::options_description general_options("General options");
    general_options.add_options()
        ("help,h", "show this help message")
        ("version,v", "show version information")
        ("config,c", po::value<std::string>(),  "optional, filepath to load configuration from. Overrides default config values")
        ("log,l", po::value<std::vector<std::string>>(),
          "Sets a custom logging filter.\n"
          "Syntax is `<target>=<level>`, e.g. -llibp2p=off.\n"
          "Log levels (most to least verbose) are trace, debug, verbose, info, warn, error, critical, off.\n"
          "By default, all targets log `info`.\n"
          "The global log level can be set with -l<level>.")

        ;

    po::options_description development_options("Additional options");
    development_options.add_options()
        ("dev", "if node run in development mode")
        ;

    // clang-format on

    po::variables_map vm;

    // first-run parse to read only general options and to lookup for "help",
    // "config" and "version". all the rest options are ignored
    try {
      po::parsed_options parsed = po::command_line_parser(argc_, argv_)
                                      .options(general_options)
                                      .allow_unregistered()
                                      .run();
      po::store(parsed, vm);
      po::notify(vm);
    } catch (const std::exception &e) {
      std::cerr << "Error: " << e.what() << '\n'
                << "Try run with option '--help' for more information" << '\n';
      return Error::CliArgsParseFailed;
    }

    po::options_description all_options;
    all_options
        .add(general_options)  //
        .add(development_options);

    if (vm.contains("help")) {
      std::cout << "JAM-node version " << buildVersion() << '\n';
      std::cout << all_options << '\n';
      return true;
    }

    if (vm.contains("version")) {
      std::cout << "JAM-node version " << buildVersion() << '\n';
      return true;
    }

    if (vm.contains("config")) {
      auto path = vm["config"].as<std::string>();
      try {
        config_file_ = YAML::LoadFile(path);
      } catch (const std::exception &exception) {
        std::cerr << "Error: Can't parse file "
                  << std::filesystem::weakly_canonical(path) << ": "
                  << exception.what() << "\n"
                  << "Option --config must be path to correct yaml-file\n"
                  << "Try run with option '--help' for more information\n";
        return Error::ConfigFileParseFailed;
      }
    }

    // try {
    //   // second-run parse to gather all known options
    //   // with reporting about any unrecognized input
    //   po::store(po::parse_command_line(argc, argv, desc), vm);
    //   po::store(parsed, vm);
    //   po::notify(vm);
    // } catch (const std::exception &e) {
    //   std::cerr << "Error: " << e.what() << '\n'
    //             << "Try run with option '--help' for more information" <<
    //             '\n';
    //   return true;
    // }

    return false;
  }

}  // namespace jam::app
