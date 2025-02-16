/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "handler.hpp"

namespace jam::metrics {

  /**
   * @brief an http server interface to expose metrics on request with custom
   * request handler
   */
  class Exposer {
   protected:
    using Acceptor = boost::asio::ip::tcp::acceptor;
    using Endpoint = boost::asio::ip::tcp::endpoint;

   public:
    using Context = boost::asio::io_context;

    struct Configuration {
      Endpoint endpoint{boost::asio::ip::address_v4::any(), 0};
    };

    /**
     * @brief sets handler and takes ownership
     */
    void setHandler(const std::shared_ptr<Handler> &handler) {
      handler_ = handler;
    }

    virtual ~Exposer() = default;

    /**
     * @brief prepare interface for AppStateManager
     */
    virtual bool prepare() = 0;

    /**
     * @brief start interface for AppStateManager
     */
    virtual bool start() = 0;

    /**
     * @brief stop interface for AppStateManager
     */
    virtual void stop() = 0;

   protected:
    std::shared_ptr<Handler> handler_;
  };

}  // namespace jam::metrics
