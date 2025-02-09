/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string>
#include <memory>
#include <dlfcn.h>
#include <cstring>
#include <cerrno>

namespace jam::modules {

class Module final : public std::enable_shared_from_this<Module> {
public:
    // Static method for Module object creation
    static std::shared_ptr<Module> create(const std::string& path, std::unique_ptr<void, decltype(&dlclose)> handle, const std::string& loader_id) {
        return std::shared_ptr<Module>(new Module(path, std::move(handle), loader_id));
    }

    // Getter for library path
    const std::string& get_path() const {
        return path_;
    }

    // Getter for loader Id
    const std::string& get_loader_id() const {
        return loader_id_;
    }

private:
    Module(const std::string& path, std::unique_ptr<void, decltype(&dlclose)> handle, const std::string& loader_id)
        : path_(path), handle_(std::move(handle)), loader_id_(loader_id) {}

    std::string path_;                                  // Library path
    std::unique_ptr<void, decltype(&dlclose)> handle_;  // Library handle
    std::string loader_id_;                             // Loader ID

    Module(const Module&) = delete;
    Module& operator=(const Module&) = delete;
    Module(Module&&) = delete;
    Module& operator=(Module&&) = delete;
};

}
