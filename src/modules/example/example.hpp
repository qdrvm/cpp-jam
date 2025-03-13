/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <modules/module.hpp>

/// @brief Magic word to check if module is JAM module actually
MODULE_C_API const uint64_t JAM_MAGIC;

/// @brief Identifier of suitable module loader
MODULE_C_API const char *loader_id();

/// @brief Returns module name and version
/// @return A string containing module name and version
MODULE_C_API const char *get_module_name_and_version();

namespace jam::modules {
  class ExampleModule;
}

MODULE_API std::weak_ptr<jam::modules::ExampleModule> query_module_instance();

MODULE_API void release_module_instance();

class BlockTree;
class ExampleModuleLoader;

namespace jam::modules {

  class ExampleModule : public Module, Singleton<ExampleModule> {  // implement by module
    using Module::Module;
    virtual ~ExampleModule() = default;
    CREATE_SHARED_METHOD(ExampleModule);
    // virtual void initialize(std::shared_ptr<BlockTree> tree,
    //                         std::weak_ptr<ExampleModuleLoader> loader) = 0;
  };

}  // namespace jam::modules
