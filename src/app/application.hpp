/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <utils/ctor_limiters.hpp>

namespace jam::app {

  /// @class Application - JAM-application interface
  class Application : private Singleton<Application> {
   public:
    virtual ~Application() = default;

    /// Runs node
    virtual void run() = 0;
  };

}  // namespace jam::app
