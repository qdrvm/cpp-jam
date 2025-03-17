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

MODULE_API std::weak_ptr<jam::modules::ExampleModule> query_module_instance(
    std::shared_ptr<jam::modules::ExampleModuleLoader> loader,
    std::shared_ptr<jam::log::LoggingSystem> block_tree) {
  return jam::modules::ExampleModule::instance
           ? jam::modules::ExampleModule::instance
           : (jam::modules::ExampleModule::instance = jam::modules::ExampleModule::create_shared(
             loader, block_tree));
}

MODULE_API void release_module_instance() {
  jam::modules::ExampleModule::instance.reset();
}
