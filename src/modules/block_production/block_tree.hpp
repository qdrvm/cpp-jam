/**
* Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <metrics/impl/session_impl.hpp>
#include <modules/module_loader.hpp>
#include <qtils/strict_sptr.hpp>

namespace jam::modules {

  struct BlockTreeLoader {
    virtual ~BlockTreeLoader() = default;

    virtual void do_request() = 0;
    virtual void do_notify()  = 0;
  };

  struct BlockTree {
    virtual ~BlockTree() = default;
    virtual void on_loaded_success() = 0;

    virtual void on_request() = 0;
    virtual void on_notify()  = 0;
  };

}  // namespace jam::modules

namespace jam::modules {

  // class BlockTree : public Singleton<BlockTree> {
  //  public:
  //   static std::shared_ptr<BlockTree> instance;
  //   CREATE_SHARED_METHOD(BlockTree);

  //   BlockTree(qtils::StrictSharedPtr<BlockTreeLoader> loader,
  //                 qtils::StrictSharedPtr<log::LoggingSystem> logging_system);

  //   qtils::StrictSharedPtr<BlockTreeLoader> loader_;
  //   log::Logger logger_;
  // };

}  // namespace jam::modules

