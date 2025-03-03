/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <unordered_map>
#include <variant>

#include <qtils/bytes_std_hash.hpp>

#include "coro/coro.hpp"
#include "coro/init.hpp"
#include "coro/io_context_ptr.hpp"
#include "snp/connections/config.hpp"
#include "snp/connections/connection_id_counter.hpp"
#include "snp/connections/connection_ptr.hpp"
#include "snp/connections/key.hpp"
#include "snp/connections/lsquic/controller.hpp"

namespace jam::log {
  class LoggingSystem;
}  // namespace jam::log

namespace jam::snp {
  class Address;
  class ConnectionsController;
}  // namespace jam::snp

namespace jam::snp::lsquic {
  class Engine;
}  // namespace jam::snp::lsquic

namespace jam::snp {
  /**
   * Initiates and accepts connections with peers.
   * Prevents duplicate connections with peers.
   */
  class Connections : public std::enable_shared_from_this<Connections>,
                      public lsquic::EngineController {
   public:
    using SelfSPtr = std::shared_ptr<Connections>;

    Connections(IoContextPtr io_context_ptr,
                std::shared_ptr<log::LoggingSystem> logsys,
                ConnectionsConfig config);

    /**
     * Set controller.
     * Start quic server and client.
     */
    static CoroOutcome<void> init(
        SelfSPtr self, std::weak_ptr<ConnectionsController> controller);

    const Key &key() const;

    /**
     * Connect or return existing connection.
     */
    static ConnectionPtrCoroOutcome connect(SelfSPtr self, Address address);

    using ServeProtocol =
        std::function<CoroOutcome<void>(ConnectionInfo, StreamPtr)>;
    /**
     * Set callback to handle protocol on server side.
     */
    static Coro<void> serve(SelfSPtr self,
                            ProtocolId protocol_id,
                            ServeProtocol serve);

    // EngineController
    void onConnectionAccept(ConnectionPtr connection) override;
    void onConnectionClose(ConnectionInfo connection_info) override;
    void onStreamAccept(ConnectionPtr connection,
                        ProtocolId protocol_id,
                        StreamPtr stream) override;

   private:
    using Connecting = std::shared_ptr<SharedFuture<ConnectionPtrOutcome>>;
    using Connected = ConnectionPtr;

    IoContextPtr io_context_ptr_;
    std::shared_ptr<log::LoggingSystem> logsys_;
    CoroInit init_;
    ConnectionsConfig config_;
    Key key_;
    std::weak_ptr<ConnectionsController> controller_;
    std::shared_ptr<lsquic::Engine> client_;
    std::optional<std::shared_ptr<lsquic::Engine>> server_;
    std::unordered_map<Key,
                       std::variant<Connecting, Connected>,
                       qtils::BytesStdHash>
        connections_;
    std::unordered_map<ProtocolId, ServeProtocol> protocols_;
    ConnectionIdCounter connection_id_counter_;
  };
}  // namespace jam::snp
