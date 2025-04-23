/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cerrno>
#include <cstring>
#include <dlfcn.h>
#include <memory>
#include <optional>
#include <string>

namespace jam::modules {

  class Module final : public std::enable_shared_from_this<Module> {
   public:
    Module(Module &&) = default;

    // Static method for Module object creation
    static std::shared_ptr<Module> create(
        const std::string &path,
        std::unique_ptr<void, int(*)(void*)> handle,
        const std::string &loader_id) {
      return std::shared_ptr<Module>(
          new Module(path, std::move(handle), loader_id));
    }

    // Getter for library path
    const std::string &get_path() const {
      return path_;
    }

    // Getter for loader Id
    const std::string &get_loader_id() const {
      return loader_id_;
    }

    // Get function address from library
    template <typename ReturnType, typename... ArgTypes>
    std::optional<ReturnType (*)(ArgTypes...)> getFunctionFromLibrary(
        const char *funcName) {
      void *funcAddr = dlsym(handle_.get(), funcName);
      if (!funcAddr) {
        return std::nullopt;
      }
      return reinterpret_cast<ReturnType (*)(ArgTypes...)>(funcAddr);
    }

   private:
    Module(const std::string &path,
           std::unique_ptr<void, int(*)(void*)> handle,
           const std::string &loader_id)
        : path_(path), handle_(std::move(handle)), loader_id_(loader_id) {}

    std::string path_;                                  // Library path
    std::unique_ptr<void, int(*)(void*)> handle_;  // Library handle
    std::string loader_id_;                             // Loader ID

    Module(const Module &) = delete;
    Module &operator=(const Module &) = delete;
    Module &operator=(Module &&) = delete;
  };

}  // namespace jam::modules
