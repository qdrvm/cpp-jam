/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "metrics/metrics.hpp"

namespace prometheus {
  class Counter;
  class Gauge;
  class Summary;
  class Histogram;
}  // namespace prometheus

namespace jam::metrics {

  class PrometheusCounter : public Counter {
    friend class PrometheusRegistry;
    prometheus::Counter &m_;

   public:
    PrometheusCounter(prometheus::Counter &m);

   public:
    void inc() override;
    void inc(double val) override;
  };

  class PrometheusGauge : public Gauge {
    friend class PrometheusRegistry;
    prometheus::Gauge &m_;

   public:
    PrometheusGauge(prometheus::Gauge &m);

   public:
    void inc() override;
    void inc(double val) override;
    void dec() override;
    void dec(double val) override;
    void set(double val) override;
  };

  class PrometheusSummary : public Summary {
    friend class PrometheusRegistry;
    prometheus::Summary &m_;

   public:
    PrometheusSummary(prometheus::Summary &m);

   public:
    void observe(const double value) override;
  };

  class PrometheusHistogram : public Histogram {
    friend class PrometheusRegistry;
    prometheus::Histogram &m_;

   public:
    PrometheusHistogram(prometheus::Histogram &m);

   public:
    void observe(const double value) override;
  };

}  // namespace jam::metrics
