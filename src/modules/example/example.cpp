/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "modules/example/example.hpp"

MODULE_C_API const uint64_t JAM_MAGIC = jam::modules::JAM_MAGIC;

std::shared_ptr<jam::modules::ExampleModule> module_instance;

MODULE_C_API const char *loader_id() {
  return "ExampleLoader";
}

MODULE_C_API const char *get_module_name_and_version() {
  return "ExampleModule v1.0";
}

std::string path_;
std::unique_ptr<void, decltype(&dlclose)> handle_{};
std::string loader_id_;

MODULE_API std::weak_ptr<jam::modules::ExampleModule> query_module_instance() {
  static bool _ = ({
    module_instance = jam::modules::ExampleModule::create_shared(
        path_,               // path
        std::move(handle_),  // handler
        loader_id_           // loader
    );
    true;
  });
  return module_instance;
}

MODULE_API void release_module_instance() {
  module_instance = nullptr;
};
