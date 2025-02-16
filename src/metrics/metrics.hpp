/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "metrics/registry.hpp"

namespace jam::metrics {
  // the function recommended to use to create a registry of the chosen
  // implementation
  std::unique_ptr<Registry> createRegistry();

  /**
   * @brief A counter metric to represent a monotonically increasing value.
   *
   * This class represents the metric type counter:
   * https://prometheus.io/docs/concepts/metric_types/#counter
   */
  class Counter {
   public:
    virtual ~Counter() = default;

    /**
     * @brief Increment the counter by 1.
     */
    virtual void inc() = 0;

    /**
     * The counter will not change if the given amount is negative.
     */
    virtual void inc(double val) = 0;
  };

  /**
   * @brief A gauge metric to represent a value that can arbitrarily go up and
   * down.
   *
   * The class represents the metric type gauge:
   * https://prometheus.io/docs/concepts/metric_types/#gauge
   */
  class Gauge {
   public:
    virtual ~Gauge() = default;

    /**
     * @brief Increment the gauge by 1.
     */
    virtual void inc() = 0;

    /**
     * @brief Increment the gauge by the given amount.
     */
    virtual void inc(double val) = 0;

    /**
     * @brief Decrement the gauge by 1.
     */
    virtual void dec() = 0;

    /**
     * @brief Decrement the gauge by the given amount.
     */
    virtual void dec(double val) = 0;

    /**
     * @brief Set the gauge to the given value.
     */
    virtual void set(double val) = 0;

    template <typename T>
    void set(T val) {
      set(static_cast<double>(val));
    }
  };

  /**
   * @brief A summary metric samples observations over a sliding window of
   * time.
   *
   * This class represents the metric type summary:
   * https://prometheus.io/docs/instrumenting/writing_clientlibs/#summary
   */
  class Summary {
   public:
    virtual ~Summary() = default;

    /**
     * @brief Observe the given amount.
     */
    virtual void observe(const double value) = 0;
  };

  /**
   * @brief A histogram metric to represent aggregatable distributions of
   * events.
   *
   * This class represents the metric type histogram:
   * https://prometheus.io/docs/concepts/metric_types/#histogram
   */
  class Histogram {
   public:
    virtual ~Histogram() = default;

    /**
     * @brief Observe the given amount.
     */
    virtual void observe(const double value) = 0;
  };
}  // namespace jam::metrics
