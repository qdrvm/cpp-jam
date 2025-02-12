/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <boost/asio/buffer.hpp>
#include <qtils/bytes.hpp>

namespace qtils {
  inline boost::asio::const_buffer asioBuffer(BytesIn s) {
    return {s.data(), s.size()};
  }

  boost::asio::mutable_buffer asioBuffer(auto &&t)
    requires(requires { BytesOut{t}; })
  {
    BytesOut s{t};
    return {s.data(), s.size()};
  }

  inline BytesIn asioBuffer(const boost::asio::const_buffer &s) {
    return {static_cast<const uint8_t *>(s.data()), s.size()};
  }

  inline BytesOut asioBuffer(const boost::asio::mutable_buffer &s) {
    return {static_cast<uint8_t *>(s.data()), s.size()};
  }
}  // namespace qtils
