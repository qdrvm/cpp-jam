/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <boost/variant.hpp>
#include <tuple>
#include <type_traits>
#include <variant>

namespace jam {

  template <typename T, typename = void>
  struct is_boost_variant : std::false_type {};

  template <typename T>
  struct is_boost_variant<T, std::void_t<typename T::types>> : std::true_type {
  };

  template <std::size_t I, typename BoostVariant>
  struct variant_alternative_boost {
    static_assert(is_boost_variant<BoostVariant>::value,
                  "Type is not a boost::variant");
    static_assert(I < std::tuple_size_v<typename BoostVariant::types>,
                  "Index out of bounds for boost::variant");
    using type = std::tuple_element_t<I, typename BoostVariant::types>;
  };

  template <std::size_t I, typename StdVariant>
  struct variant_alternative_std {
    static_assert(I < std::variant_size_v<StdVariant>,
                  "Index out of bounds for std::variant");
    using type = std::variant_alternative_t<I, StdVariant>;
  };

  template <std::size_t I, typename Variant>
  struct variant_alternative
      : std::conditional_t<is_boost_variant<Variant>::value,
                           variant_alternative_boost<I, Variant>,
                           variant_alternative_std<I, Variant>> {};

  template <std::size_t I, typename Variant>
  using variant_alternative_t = typename variant_alternative<I, Variant>::type;

}  // namespace jam
