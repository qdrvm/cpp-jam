/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "app/state_manager.hpp"

#include <condition_variable>
#include <csignal>
#include <mutex>
#include <queue>

#include "utils/ctor_limiters.hpp"

namespace soralog {
  class Logger;
}  // namespace soralog
namespace morum::log {
  class LoggingSystem;
}  // namespace morum::log

namespace morum::app {

  class StateManagerImpl final
      : Singleton<StateManager>,
        public StateManager,
        public std::enable_shared_from_this<StateManagerImpl> {
   public:
    StateManagerImpl(std::shared_ptr<log::LoggingSystem> logging_system);

    ~StateManagerImpl() override;

    void atPrepare(OnPrepare &&cb) override;
    void atLaunch(OnLaunch &&cb) override;
    void atShutdown(OnShutdown &&cb) override;

    void run() override;
    void shutdown() override;

    State state() const override {
      return state_;
    }

   protected:
    void reset();

    void doPrepare() override;
    void doLaunch() override;
    void doShutdown() override;

   private:
    static std::weak_ptr<StateManagerImpl> wp_to_myself;

    static std::atomic_bool shutting_down_signals_enabled;
    static void shuttingDownSignalsEnable();
    static void shuttingDownSignalsDisable();
    static void shuttingDownSignalsHandler(int);

    static std::atomic_bool log_rotate_signals_enabled;
    static void logRotateSignalsEnable();
    static void logRotateSignalsDisable();
    static void logRotateSignalsHandler(int);

    void shutdownRequestWaiting();

    std::shared_ptr<soralog::Logger> logger_;
    std::shared_ptr<log::LoggingSystem> logging_system_;

    std::atomic<State> state_ = State::Init;

    std::recursive_mutex mutex_;

    std::mutex cv_mutex_;
    std::condition_variable cv_;

    std::queue<OnPrepare> prepare_;
    std::queue<OnLaunch> launch_;
    std::queue<OnShutdown> shutdown_;
  };

}  // namespace morum::app
