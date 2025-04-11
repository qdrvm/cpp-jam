/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jam_types/types.tmp.hpp>
#include <metrics/impl/session_impl.hpp>
#include <modules/module_loader.hpp>
#include <modules/shared/networking_types.tmp.hpp>
#include <modules/shared/synchronizer_types.tmp.hpp>
#include <qtils/create_smart_pointer_macros.hpp>
#include <qtils/strict_sptr.hpp>

namespace jam::modules {

  struct NetworkingLoader {
    virtual ~NetworkingLoader() = default;

    virtual void dispatch_block_announce(
        std::shared_ptr<const messages::BlockAnnounceMessage> msg) = 0;

    virtual void dispatch_block_response(
        std::shared_ptr<const messages::BlockResponseMessage> msg) = 0;
  };

  struct Networking {
    virtual ~Networking() = default;

    virtual void on_loaded_success() = 0;

    virtual void on_loading_is_finished() = 0;

    virtual void on_block_request(
        std::shared_ptr<const messages::BlockRequestMessage> msg) = 0;
  };

}  // namespace jam::modules

namespace jam::modules {

  class NetworkingImpl final : public Singleton<Networking>, public Networking {
   public:
    static std::shared_ptr<Networking> instance;
    CREATE_SHARED_METHOD(NetworkingImpl);

    NetworkingImpl(qtils::StrictSharedPtr<NetworkingLoader> loader,
                   qtils::StrictSharedPtr<log::LoggingSystem> logging_system);

    void on_loaded_success() override;

    void on_loading_is_finished() override;

    void on_block_request(
        std::shared_ptr<const messages::BlockRequestMessage> msg) override;

   private:
    qtils::StrictSharedPtr<NetworkingLoader> loader_;
    log::Logger logger_;
  };

}  // namespace jam::modules
