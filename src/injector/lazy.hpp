/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <boost/di/extension/injections/lazy.hpp>

namespace jam {

  template <typename T>
  using Lazy = boost::di::extension::lazy<T>;

  template <typename T>
  using LazyRef = boost::di::extension::lazy<T &>;

  template <typename T>
  using LazyCRef = boost::di::extension::lazy<const T &>;

  template <typename T>
  using LazySPtr = boost::di::extension::lazy<std::shared_ptr<T>>;

  template <typename T>
  using LazyUPtr = boost::di::extension::lazy<std::unique_ptr<T>>;

}  // namespace jam
