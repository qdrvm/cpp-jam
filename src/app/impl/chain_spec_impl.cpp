/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "app/impl/chain_spec_impl.hpp"

#include <charconv>
// #include <libp2p/multi/multiaddress.hpp>
// #include <libp2p/peer/peer_id.hpp>
#include <sstream>
#include <system_error>

#include <app/configuration.hpp>
#include <qtils/hex.hpp>

// #include "assets/embedded_chainspec.hpp"
// #include "common/hexutil.hpp"
// #include "common/visitor.hpp"

OUTCOME_CPP_DEFINE_CATEGORY(jam::app, ChainSpecImpl::Error, e) {
  using E = jam::app::ChainSpecImpl::Error;
  switch (e) {
    case E::MISSING_ENTRY:
      return "A required entry is missing in the config file";
    case E::MISSING_PEER_ID:
      return "Peer id is missing in a multiaddress provided in the config file";
    case E::PARSER_ERROR:
      return "Internal parser error";
    case E::NOT_IMPLEMENTED:
      return "Known entry name, but parsing not implemented";
  }
  return "Unknown error in ChainSpecImpl";
}

namespace jam::app {

  namespace pt = boost::property_tree;

  ChainSpecImpl::ChainSpecImpl(qtils::SharedRef<log::LoggingSystem> logsys,
                               qtils::SharedRef<Configuration> configuration)
      : log_(logsys->getLogger("ChainSpec", "application")) {
    qtils::raise_on_err(loadFromJson(configuration->specFile()));
  }

  outcome::result<void> ChainSpecImpl::loadFromJson(
      const std::filesystem::path &file_path) {
    pt::ptree tree;
    try {
      // if (auto asset = assets::getEmbeddedChainspec(file_path)) {
      //   std::istringstream s{std::string{*asset}};
      //   pt::json_parser::read_json(s, tree);
      // } else {
      pt::read_json(file_path, tree);
      // }
    } catch (pt::json_parser_error &e) {
      log_->error(
          "Parser error: {}, line {}: {}", e.filename(), e.line(), e.message());
      return Error::PARSER_ERROR;
    }

    OUTCOME_TRY(loadFields(tree));
    OUTCOME_TRY(loadGenesis(tree));
    OUTCOME_TRY(loadBootNodes(tree));

    return outcome::success();
  }

  outcome::result<void> ChainSpecImpl::loadFields(
      const boost::property_tree::ptree &tree) {
    OUTCOME_TRY(id, ensure("id", tree.get_child_optional("id")));
    id_ = id.get<std::string>("");

    return outcome::success();
  }

  outcome::result<void> ChainSpecImpl::loadGenesis(
      const boost::property_tree::ptree &tree) {
    try {
      auto genesis_header_hex = tree.get<std::string>("genesis_header");
      OUTCOME_TRY(genesis_header_encoded,
                  qtils::ByteVec::fromHex(genesis_header_hex));
      genesis_header_ = std::move(genesis_header_encoded);
    } catch (const boost::property_tree::ptree_error &e) {
      SL_CRITICAL(log_,
                  "Failed to read genesis block header from chain spec: {}",
                  e.what());
    }
    return outcome::success();


    // auto read_key_block = [](const auto &tree,
    //                          qtils::ByteVec &data) -> outcome::result<void> {
    //   // for (const auto &[key, value] : tree) {
    //   //   // get rid of leading 0x for key and value and unhex
    //   //   OUTCOME_TRY(key_processed, qtils::unhex0x(key));
    //   //   OUTCOME_TRY(value_processed, qtils::unhex0x(value.data()));
    //   //   data.emplace_back(std::move(key_processed),
    //   //   std::move(value_processed));
    //   // }
    //   return outcome::success();
    // };
    //
    // OUTCOME_TRY(read_key_block(genesis_header, genesis_header_));
    //
    // return outcome::success();
  }

  outcome::result<void> ChainSpecImpl::loadBootNodes(
      const boost::property_tree::ptree &tree) {
    // OUTCOME_TRY(boot_nodes,
    //             ensure("bootNodes", tree.get_child_optional("bootNodes")));
    // for (auto &v : boot_nodes) {
    //   if (auto ma_res = libp2p::multi::Multiaddress::create(v.second.data()))
    //   {
    //     auto &&multiaddr = ma_res.value();
    //     if (auto peer_id_base58 = multiaddr.getPeerId();
    //         peer_id_base58.has_value()) {
    //       OUTCOME_TRY(libp2p::peer::PeerId::fromBase58(peer_id_base58.value()));
    //       boot_nodes_.emplace_back(std::move(multiaddr));
    //     } else {
    //       return Error::MISSING_PEER_ID;
    //     }
    //   } else {
    //     log_->warn("Unsupported multiaddress '{}'. Ignoring that boot node",
    //                v.second.data());
    //   }
    // }
    return outcome::success();
  }

}  // namespace jam::app
