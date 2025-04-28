/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <thread>

#include <qtils/create_smart_pointer_macros.hpp>
#include <qtils/strict_sptr.hpp>

#include "coro/coro.hpp"
#include "coro/io_context_ptr.hpp"
#include "log/logger.hpp"
#include "modules/networking/interfaces.hpp"
#include "snp/connections/config.hpp"
#include "snp/connections/controller.hpp"
#include "snp/connections/message_size.hpp"
#include "snp/connections/protocol_id.hpp"
#include "snp/connections/stream_ptr.hpp"
#include "utils/ctor_limiters.hpp"

namespace jam::snp {
  class Connections;
}  // namespace jam::snp

namespace jam::modules {
  enum class NetworkingError : uint8_t {
    NETWORKING_DESTROYED,
    END_OF_STREAM,
  };
  Q_ENUM_ERROR_CODE(NetworkingError) {
    using E = decltype(e);
    switch (e) {
      case E::NETWORKING_DESTROYED:
        return "NetworkingImpl destroyed";
      case E::END_OF_STREAM:
        return "end of stream";
    }
  }

  class NetworkingImpl final : public Singleton<Networking>,
                               public Networking,
                               public snp::ConnectionsController {
   public:
    using SelfSPtr = std::shared_ptr<NetworkingImpl>;
    static SelfSPtr instance;
    CREATE_SHARED_METHOD(NetworkingImpl);

   private:
    NetworkingImpl(qtils::StrictSharedPtr<NetworkingLoader> loader,
                   qtils::StrictSharedPtr<log::LoggingSystem> logging_system);

   public:
    ~NetworkingImpl();

    // Networking
    void on_loaded_success() override;
    void on_loading_is_finished() override;
    void on_block_request(
        std::shared_ptr<const messages::SendBlockRequest> msg) override;

    // ConnectionsController
    void onOpen(snp::Key key) override;
    void onClose(snp::Key key) override;
    void onConnectionChange(snp::ConnectionPtr connection) override;

   private:
    template <typename T>
    static CoroOutcome<T> read(SelfSPtr &self,
                               snp::StreamPtr stream,
                               qtils::Bytes buffer);
    template <typename T>
    static CoroOutcome<void> write(SelfSPtr &self,
                                   snp::StreamPtr stream,
                                   qtils::Bytes buffer,
                                   const T &message);
    template <typename Response, typename Request>
    static CoroOutcome<Response> sendRequest(SelfSPtr &self,
                                             PeerId peer_id,
                                             snp::ProtocolId protocol,
                                             const Request &request);
    static Coro<void> handleBlockAnnouncement(SelfSPtr &self,
                                              snp::Key key,
                                              snp::StreamPtr stream);

    qtils::StrictSharedPtr<NetworkingLoader> loader_;
    log::Logger logger_;
    IoContextPtr io_context_ptr_;
    std::thread io_thread_;
    qtils::StrictSharedPtr<snp::Connections> connections_;
  };

}  // namespace jam::modules
