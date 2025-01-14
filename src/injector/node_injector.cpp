/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#define BOOST_DI_CFG_DIAGNOSTICS_LEVEL 2
#define BOOST_DI_CFG_CTOR_LIMIT_SIZE 32

#include "injector/node_injector.hpp"

#include <boost/di.hpp>
#include <boost/di/extension/scopes/shared.hpp>

#include "app/configuration.hpp"
#include "app/impl/state_manager_impl.hpp"
#include "app/impl/application_impl.hpp"
#include "log/logger.hpp"

#include "injector/bind_by_lambda.hpp"
#include "log/configurator.hpp"
#include "log/logger.hpp"

namespace {
  template <class T>
  using sptr = std::shared_ptr<T>;

  template <class T>
  using uptr = std::unique_ptr<T>;

  namespace di = boost::di;
  namespace fs = std::filesystem;
  using namespace jam;  // NOLINT

  template <typename C>
  auto useConfig(C c) {
    return boost::di::bind<std::decay_t<C>>().to(
        std::move(c))[boost::di::override];
  }

  using injector::bind_by_lambda;

  template <typename... Ts>
  auto makeApplicationInjector(std::shared_ptr<log::LoggingSystem> logsys,
                               std::shared_ptr<app::Configuration> config,
                               Ts &&...args) {
    // clang-format off
    return di::make_injector(
            // bind configs
            // useConfig(ws_config),

            // inherit host injector
            // libp2p::injector::makeHostInjector(
            //     libp2p::injector::useWssPem(config->nodeWssPem()),
            //     libp2p::injector::useSecurityAdaptors<
            //         libp2p::security::Noise>()[di::override]),

            // inherit kademlia injector
            // libp2p::injector::makeKademliaInjector(),
            // bind_by_lambda<libp2p::protocol::kademlia::Config>(
            //     [random_walk{config->getRandomWalkInterval()}](
            //         const auto &injector) {
            //       return get_kademlia_config(
            //         injector.template create<const blockchain::GenesisBlockHash &>(),
            //         injector.template create<const application::ChainSpec &>(), random_walk);
            //     })[boost::di::override],

            di::bind<app::StateManager>.template to<app::StateManagerImpl>(),
            di::bind<app::Application>.template to<app::ApplicationImpl>(),

            di::bind<app::Configuration>.to(config),
            di::bind<log::LoggingSystem>.to(logsys),

            // di::bind<api::Listener *[]>()  // NOLINT
            //     .template to<api::WsListenerImpl>(),

            // hardfix for Mac clang
            // di::bind<metrics::Session::Configuration>.to(
            //     [](const auto &injector) {
            //       return metrics::Session::Configuration{};
            //     }),

            // user-defined overrides...
            std::forward<decltype(args)>(args)...);
    // clang-format on
  }

  template <typename... Ts>
  auto makeNodeInjector(std::shared_ptr<log::LoggingSystem> logsys,
                        std::shared_ptr<app::Configuration> config,
                        Ts &&...args) {
    return di::make_injector<boost::di::extension::shared_config>(
        makeApplicationInjector(std::move(logsys), std::move(config)),

        // user-defined overrides...
        std::forward<decltype(args)>(args)...);
  }

}  // namespace

namespace jam::injector {

  class NodeInjectorImpl {
   public:
    using Injector =
        decltype(makeNodeInjector(std::shared_ptr<log::LoggingSystem>(),
                                  std::shared_ptr<app::Configuration>()));

    explicit NodeInjectorImpl(Injector injector)
        : injector_{std::move(injector)} {}
    Injector injector_;
  };

  NodeInjector::NodeInjector(std::shared_ptr<log::LoggingSystem> logsys,
                             std::shared_ptr<app::Configuration> config)
      : pimpl_{std::make_unique<NodeInjectorImpl>(
            makeNodeInjector(std::move(logsys), std::move(config)))} {}

  std::shared_ptr<app::Application> NodeInjector::injectApplication() {
    return pimpl_->injector_
        .template create<std::shared_ptr<app::Application>>();
  }

}  // namespace jam::injector
