/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "injector/node_injector.hpp"
#include "se/impl/common.hpp"
#include <functional>
#include <tuple>
#include "modules/module.hpp"

namespace jam::loaders {

    template <typename T>
    struct ResponseContext {
        enum class Error : uint8_t {
            REQUEST_RESPONSE_DELETED = 1,
        };

        using Response = outcome::result<T>;
        virtual ~ResponseContext() = default;
        virtual void on_response(Response response) = 0;
    };

    template <typename T, typename FRespCb, typename... Args>
    struct RequestResponse final : public ResponseContext<T>, se::utils::NoCopy {
        using typename ResponseContext<T>::Response;

        RequestResponse(FRespCb &&on_response_cb, Args&&... args)
            : on_response_cb_(std::move(on_response_cb)), args_(std::forward<Args>(args)...) {}

        ~RequestResponse() override {
            if (on_response_cb_) {
                (*on_response_cb_)(args(), outcome::failure(std::error_code(
                    static_cast<int>(ResponseContext<T>::Error::REQUEST_RESPONSE_DELETED))));
            }
        }

        void on_response(Response response) override {
            if (on_response_cb_) {
                (*on_response_cb_)(args(), std::move(response));
                on_response_cb_ = std::nullopt;
            }
        }

        std::tuple<Args...> &args() {
            return args_;
        }

        const std::tuple<Args...> &args() const {
            return args_;
        }

    private:
        std::optional<FRespCb> on_response_cb_;
        std::tuple<Args...> args_;
    };

    template <typename T, typename FRespCb, typename... Args>
    inline auto make_request(FRespCb &&on_response_cb, Args&&... args) {
        return RequestResponse<T, std::decay_t<FRespCb>, std::decay_t<Args>...>(
            std::forward<FRespCb>(on_response_cb), 
            std::forward<Args>(args)...
        );
    }

    /* Example:
    auto r2 = jam::loaders::make_request<std::string>(
            [](const std::tuple<std::string, int>& args, outcome::result<std::string> resp) {
                if(resp) {
                    std::cout << "Успех: " << resp.value() << std::endl;
                } else {
                    std::cout << "Ошибка при обработке "
                            << std::get<0>(args) << " (код: " 
                            << std::get<1>(args) << ")" << std::endl;
                }
            },
            "запрос данных", 42
        );
        
        r2.on_response(outcome::result<std::string>("результат")); 
    */

}  // namespace jam::loaders
