/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>

#define MAKE_SHARED_(x_, ...)                                 \
  x_ {                                                        \
    std::make_shared<decltype(x_)::element_type>(__VA_ARGS__) \
  }

#define MAKE_SHARED_T(T, ...) std::make_shared<T::element_type>(__VA_ARGS__)
