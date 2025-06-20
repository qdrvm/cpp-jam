/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <modules/example/example.hpp>

#define MODULE_C_API extern "C" __attribute__((visibility("default")))
#define MODULE_API __attribute__((visibility("default")))

MODULE_C_API const char *loader_id() {
  return "ExampleLoader";
}

MODULE_C_API const char *module_info() {
  return "ExampleModule v1.0";
}

static std::shared_ptr<jam::modules::ExampleModule> module_instance;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type-c-linkage"

MODULE_C_API std::weak_ptr<jam::modules::ExampleModule> query_module_instance(
    jam::modules::ExampleModuleLoader &loader,
    std::shared_ptr<jam::log::LoggingSystem> logger) {
  if (!module_instance) {
    module_instance = std::make_shared<jam::modules::ExampleModuleImpl>(
        loader, std::move(logger));
  }
  return module_instance;
}

MODULE_C_API void release_module_instance() {
  module_instance.reset();
}

#pragma GCC diagnostic pop
