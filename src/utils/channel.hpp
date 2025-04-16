/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <type_traits>

namespace jam {

  template <typename T>
  struct Channel {
    struct _Receiver;
    struct _Sender;

    struct _Receiver {
      using Other = _Sender;
    };
    struct _Sender {
      using Other = _Receiver;
    };

    template <typename Opp>
    struct Endpoint : se::utils::NoCopy {
      static_assert(std::is_same_v<Opp, _Receiver>
                        || std::is_same_v<Opp, _Sender>,
                    "Incorrect type");
      static constexpr bool IsReceiver = std::is_same_v<Opp, _Receiver>;
      static constexpr bool IsSender = std::is_same_v<Opp, _Sender>;

      Endpoint(Endpoint &&other) requires(IsReceiver) {
        context_.exclusiveAccess([&](auto &my_context) {
            Endpoint<typename Opp::Other> *opp = nullptr;
            while (other.context_.exclusiveAccess([&](auto &other_context) {
                if (other_context.opp_) {
                    if (!other_context.opp_->register_opp(*this)) {
                        return true;
                    }
                    opp = other_context.opp_;
                    other_context.opp_ = nullptr;
                }
                return false;
            }));
            my_context.opp_ = opp;
        });
      }

      Endpoint(Endpoint &&other) requires(IsSender) {
        context_.exclusiveAccess([&](auto &my_context) {
            my_context.opp_ = other.context_.exclusiveAccess([&](auto &other_context) {
                Endpoint<typename Opp::Other> *opp = nullptr;
                if (other_context.opp_) {
                    other_context.opp_->register_opp(*this);
                    opp = other_context.opp_;
                    other_context.opp_ = nullptr;
                }
                return opp;
            });
        });
      }

      Endpoint &operator=(Endpoint &&other) requires(IsReceiver) {
        if (this != &other) {
          context_.exclusiveAccess([&](auto &my_context) {
              Endpoint<typename Opp::Other> *opp = nullptr;
              while (other.context_.exclusiveAccess([&](auto &other_context) {
                  if (other_context.opp_) {
                      if (!other_context.opp_->register_opp(*this)) {
                          return true;
                      }
                      opp = other_context.opp_;
                      other_context.opp_ = nullptr;
                  }
                  return false;
              }));
              my_context.opp_ = opp;
          });
        }
        return *this;
      }

      Endpoint &operator=(Endpoint &&other) requires(IsSender) {
        if (this != &other) {
          context_.exclusiveAccess([&](auto &my_context) {
              my_context.opp_ = other.context_.exclusiveAccess([&](auto &other_context) {
                  Endpoint<typename Opp::Other> *opp = nullptr;
                  if (other_context.opp_) {
                      other_context.opp_->register_opp(*this);
                      opp = other_context.opp_;
                      other_context.opp_ = nullptr;
                  }
                  return opp;
              });
          });
        }
        return *this;
      }

      bool register_opp(Endpoint<typename Opp::Other> &opp) requires(IsReceiver) {
        return context_.exclusiveAccess([&](auto &context) { 
            context.opp_ = &opp; 
            return true;
            });
        
      }

      bool register_opp(Endpoint<typename Opp::Other> &opp) requires(IsSender) {
        return context_
            .try_exclusiveAccess([&](auto &context) {
                context.opp_ = &opp; 
            })
            .has_value();
      }

      bool unregister_opp(Endpoint<typename Opp::Other> &opp)
        requires(IsReceiver)
      {
        return context_.exclusiveAccess([&](auto &context) {
          assert(context.opp_ == &opp);
          context.opp_ = nullptr;
            return true;
        });
      }

      bool unregister_opp(Endpoint<typename Opp::Other> &opp)
        requires(IsSender)
      {
        return context_
            .try_exclusiveAccess([&](auto &context) {
              assert(context.opp_ == &opp);
              context.opp_ = nullptr;
            })
            .has_value();
      }

      ~Endpoint()
        requires(IsSender)
      {
        context_.exclusiveAccess([&](auto &context) {
          if (context.opp_) {
            context.opp_->unregister_opp(*this);
            context.opp_->event_.set();
            context.opp_ = nullptr;
          }
        });
      }


      ~Endpoint()
        requires(IsReceiver)
      {
        while (context_.exclusiveAccess([&](auto &context) {
          if (context.opp_) {
            if (!context.opp_->unregister_opp(*this)) {
              return true;
            }
            context.opp_ = nullptr;
          }
          return false;
        }));
      }

      void set(T &&t)
        requires(IsSender)
      {
        context_.exclusiveAccess([&](auto &context) {
          if (context.opp_) {
            context.opp_->context_.exclusiveAccess(
                [&](auto &c) { c.data_ = std::move(t); });
            context.opp_->event_.set();
          }
        });
      }

      void set(T &t)
        requires(IsSender)
      {
        context_.exclusiveAccess([&](auto &context) {
          if (context.opp_) {
            context.opp_->context_.exclusiveAccess(
                [&](auto &c) { c.data_ = t; });
            context.opp_->event_.set();
          }
        });
      }

      std::optional<T> wait()
        requires(IsReceiver)
      {
        event_.wait();
        return context_.exclusiveAccess(
            [&](auto &context) { return std::move(context.data_); });
      }

     private:
      friend struct Endpoint<typename Opp::Other>;
      struct SafeContext {
        std::conditional_t<std::is_same_v<Opp, _Receiver>,
                           std::optional<T>,
                           std::monostate>
            data_;
        Endpoint<typename Opp::Other> *opp_ = nullptr;
      };

      std::conditional_t<std::is_same_v<Opp, _Receiver>,
                         jam::se::utils::WaitForSingleObject,
                         std::monostate>
          event_;
      jam::se::utils::SafeObject<SafeContext, std::mutex> context_;
    };

    using Receiver = Endpoint<_Receiver>;
    using Sender = Endpoint<_Sender>;

    inline std::pair<Receiver, Sender> create_channel() {
      using C = Channel<T>;
      C::Receiver r;
      C::Sender s;

      r.register_opp(s);
      s.register_opp(r);
      return std::make_pair(std::move(r), std::move(s));
    }
  };

}  // namespace jam
