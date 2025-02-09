/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "modules/module.hpp"
#include "injector/node_injector.hpp"

namespace jam::loaders {

class Loader {
public:
    Loader(const Loader&) = delete;
    Loader& operator=(const Loader&) = delete;

    virtual ~Loader() = default;
    virtual void start() = 0;

protected:
    injector::NodeInjector injector_;
    modules::Module module_;
};
}
