/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

namespace jam::storage::face {
  template <typename T>
  struct OwnedOrViewTrait;

  template <typename T>
  using OwnedOrView = typename OwnedOrViewTrait<T>::type;
}  // namespace jam::storage::face
