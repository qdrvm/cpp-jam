/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <filesystem>
#include <string>

#include <filesystem>
#include <boost/asio/ip/tcp.hpp>
#include <utils/ctor_limiters.hpp>

namespace jam::app {
  class Configuration final : Singleton<Configuration> {
   public:
    using Endpoint = boost::asio::ip::tcp::endpoint;

    struct MetricsConfig {
      Endpoint endpoint;
      std::optional<bool> enabled;
    };

    Configuration();

    // /// Generate yaml-file with actual config
    // virtual void generateConfigFile() const = 0;

    [[nodiscard]] virtual const std::string &nodeVersion() const;
    [[nodiscard]] virtual const std::string &nodeName() const;
    [[nodiscard]] virtual const std::filesystem::path &basePath() const;
    [[nodiscard]] virtual const std::filesystem::path &modulesDir() const;

    [[nodiscard]] virtual const MetricsConfig &metrics() const;

   private:
    friend class Configurator;  // for external configure

    std::string version_;
    std::string name_;
    std::filesystem::path base_path_;
    std::filesystem::path modules_dir_;

    MetricsConfig metrics_;
  };

}  // namespace jam::app
