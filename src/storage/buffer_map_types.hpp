/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

/**
 * This file contains convenience typedefs for interfaces from face/, as they
 * are mostly used with ByteVec key and value types
 */

#include <qtils/byte_vec.hpp>
#include <qtils/byte_vec_or_view.hpp>

#include "storage/face/batch_writeable.hpp"
#include "storage/face/generic_maps.hpp"
#include "storage/face/write_batch.hpp"

namespace jam::storage::face {

  template <>
  struct OwnedOrViewTrait<qtils::ByteVec> {
    using type = qtils::ByteVecOrView;
  };

  template <>
  struct ViewTrait<qtils::ByteVec> {
    using type = qtils::ByteView;
  };

}  // namespace jam::storage::face

namespace jam::storage {

  using qtils::ByteVec;
  using qtils::ByteVecOrView;
  using qtils::ByteView;

  using BufferBatch = face::WriteBatch<ByteVec, ByteVec>;

  using BufferStorage = face::GenericStorage<ByteVec, ByteVec>;

  using BufferStorageCursor = face::MapCursor<ByteVec, ByteVec>;

}  // namespace jam::storage
