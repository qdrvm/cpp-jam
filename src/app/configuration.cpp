/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "app/configuration.hpp"

namespace jam::app {

    std::string_view Configuration::nodeVersion() const
        {
      return "version";
      }

    std::string_view Configuration::nodeName() const{
      return "node";
    }

}  // namespace jam::app