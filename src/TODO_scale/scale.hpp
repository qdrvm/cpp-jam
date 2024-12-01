/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <tuple>
#include <variant>

#include <scale/scale.hpp>

namespace scale {

//  /**
//   * @brief scale-encodes std::variant value
//   * @tparam T type list
//   * @param v value to encode
//   * @return reference to stream
//   */
//  template <uint8_t I, class... Ts>
//  void tryEncodeAsOneOfVariant(ScaleEncoderStream &s,
//                               const std::variant<Ts...> &v) {
//    using T = std::tuple_element_t<I, std::tuple<Ts...>>;
//    if (v.index() == I) {
//      s << I << std::get<T>(v);
//      return;
//    }
//    if constexpr (sizeof...(Ts) > I + 1) {
//      tryEncodeAsOneOfVariant<I + 1>(s, v);
//    }
//  }
//
//  /**
//   * @brief scale-encodes std::variant value
//   * @tparam Ts type list of variant
//   * @param s output steam for encoded data
//   * @param v value to encode
//   * @return reference to stream
//   */
//  template <class... Ts>
//  ScaleEncoderStream &operator<<(ScaleEncoderStream &s,
//                                 const std::variant<Ts...> &v) {
//    tryEncodeAsOneOfVariant<0>(s, v);
//    return s;
//  }
//
//  template <size_t I, class... Ts>
//  void tryDecodeAsOneOfVariant(ScaleDecoderStream &s,
//                               std::variant<Ts...> &v,
//                               size_t i) {
//    using T = std::remove_const_t<std::tuple_element_t<I, std::tuple<Ts...>>>;
//    static_assert(std::is_default_constructible_v<T>);
//    if (I == i) {
//      T val;
//      s >> val;
//      v = std::forward<T>(val);
//      return;
//    }
//    if constexpr (sizeof...(Ts) > I + 1) {
//      tryDecodeAsOneOfVariant<I + 1>(s, v, i);
//    }
//  }
//
//  /**
//   * @brief scale-decodes std::variant value
//   * @tparam Ts type list of variant
//   * @param s input steam of decoding data
//   * @param v value to decode to
//   * @return reference to stream
//   */
//  template <class... Ts>
//  ScaleDecoderStream &operator>>(ScaleDecoderStream &s,
//                                 std::variant<Ts...> &v) {
//    // first byte means type index
//    uint8_t type_index = 0u;
//    s >> type_index;  // decode type index
//
//    // ensure that index is in [0, types_count)
//    if (type_index >= sizeof...(Ts)) {
//      raise(DecodeError::WRONG_TYPE_INDEX);
//    }
//
//    tryDecodeAsOneOfVariant<0>(s, v, type_index);
//    return s;
//  }

}


template <std::size_t I, typename V>
struct boost_variant_alternative;

template <std::size_t I, typename... Ts>
struct boost_variant_alternative<I, boost::variant<Ts...>> {
    static_assert(I < sizeof...(Ts), "Index out of bounds for boost::variant");
    using type = std::tuple_element_t<I, std::tuple<Ts...>>;
};

template <std::size_t I, typename V>
using boost_variant_alternative_t = typename boost_variant_alternative<I, V>::type;
