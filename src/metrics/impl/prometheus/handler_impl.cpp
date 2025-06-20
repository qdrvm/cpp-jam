/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "metrics/impl/prometheus/handler_impl.hpp"

#include <prometheus/text_serializer.h>

#include "log/logger.hpp"
#include "registry_impl.hpp"
#include "utils/retain_if.hpp"
// #include "utils/wptr.hpp"

using prometheus::Collectable;
using prometheus::MetricFamily;
using prometheus::TextSerializer;

std::vector<MetricFamily> CollectMetrics(
    const std::vector<std::weak_ptr<Collectable>> &collectables) {
  auto collected_metrics = std::vector<MetricFamily>{};

  for (auto &&wcollectable : collectables) {
    auto collectable = wcollectable.lock();
    if (!collectable) {
      continue;
    }

    auto &&metrics = collectable->Collect();
    collected_metrics.insert(collected_metrics.end(),
                             std::make_move_iterator(metrics.begin()),
                             std::make_move_iterator(metrics.end()));
  }

  return collected_metrics;
}

namespace jam::metrics {

  PrometheusHandler::PrometheusHandler(
      std::shared_ptr<log::LoggingSystem> logsys)
      : logger_{logsys->getLogger("PrometheusHandler", "metrics")} {}

  void PrometheusHandler::onSessionRequest(Session::Request request,
                                           std::shared_ptr<Session> session) {
    std::vector<MetricFamily> metrics;

    {
      std::lock_guard<std::mutex> lock{collectables_mutex_};
      metrics = CollectMetrics(collectables_);
    }

    const TextSerializer serializer;

    [[maybe_unused]] auto size =
        writeResponse(session, request, serializer.Serialize(metrics));
  }

  std::size_t PrometheusHandler::writeResponse(std::shared_ptr<Session> session,
                                               const Session::Request &request,
                                               const std::string &body) {
    Session::Response res{boost::beast::http::status::ok, request.version()};
    res.set(boost::beast::http::field::content_type,
            "text/plain; charset=utf-8");
    res.content_length(body.size());
    res.keep_alive(true);
    res.body() = body;
    session->respond(res);
    return body.size();
  }

  // it is called once on init
  void PrometheusHandler::registerCollectable(Registry &registry) {
    auto *pregistry = dynamic_cast<PrometheusRegistry *>(&registry);
    if (pregistry) {
      registerCollectable(pregistry->registry());
    }
  }

  void PrometheusHandler::registerCollectable(
      const std::weak_ptr<Collectable> &collectable) {
    std::lock_guard<std::mutex> lock{collectables_mutex_};
    cleanupStalePointers(collectables_);
    collectables_.push_back(collectable);
  }

  void PrometheusHandler::removeCollectable(
      const std::weak_ptr<Collectable> &collectable) {
    std::lock_guard<std::mutex> lock{collectables_mutex_};
    retain_if(collectables_, [&](const std::weak_ptr<Collectable> &candidate) {
      // Check if wptrs references to the same data
      return not candidate.owner_before(collectable)
         and not collectable.owner_before(candidate);
    });
  }

  void PrometheusHandler::cleanupStalePointers(
      std::vector<std::weak_ptr<Collectable>> &collectables) {
    retain_if(collectables, [](const std::weak_ptr<Collectable> &candidate) {
      return not candidate.expired();
    });
  }

}  // namespace jam::metrics
