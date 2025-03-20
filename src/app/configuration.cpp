/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "app/configuration.hpp"

namespace jam::app {

  Configuration::Configuration()
      : version_("undefined"), name_("unnamed"), metrics_endpoint_() {}

  std::string Configuration::nodeVersion() const {
    return version_;
  }

  std::string Configuration::nodeName() const {
    return name_;
  }

  std::filesystem::path Configuration::basePath() const {
    return base_path_;
  }

  std::filesystem::path Configuration::modulesDir() const {
    return modules_dir_;
  }

  std::optional<Configuration::Endpoint> Configuration::metricsEndpoint()
      const {
    return metrics_endpoint_;
  }

}  // namespace jam::app
