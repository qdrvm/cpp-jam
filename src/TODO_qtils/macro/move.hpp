/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <utility>

#define MOVE(x)  \
  x {            \
    std::move(x) \
  }

#define MOVE_(x) \
  x##_ {         \
    std::move(x) \
  }
