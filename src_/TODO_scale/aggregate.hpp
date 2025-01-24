/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <type_traits>

namespace scale::detail {

  struct ArgHelper {
    template <typename T>
    operator T() const {
      return T{};
    }
  };

  template <typename T, std::size_t... Indices>
    requires std::is_aggregate_v<T>
  constexpr bool is_constructible_with_n_def_args_impl(
      std::index_sequence<Indices...>) {
    return std::is_constructible_v<T,
                                   decltype((void(Indices), ArgHelper{}))...>;
  }

  template <typename T, size_t N>
  constexpr bool is_constructible_with_n_def_args_v =
      is_constructible_with_n_def_args_impl<T>(std::make_index_sequence<N>{});

  template <typename T, int N = -1>
    requires std::is_aggregate_v<T>
  struct field_number_of_impl
      : std::integral_constant<
            int,
            std::conditional_t<
                std::is_empty_v<T>,
                std::integral_constant<int, 0>,
                std::conditional_t<is_constructible_with_n_def_args_v<T, N + 1>,
                                   field_number_of_impl<T, N + 1>,
                                   std::integral_constant<int, N>>>::value> {};

  template <typename T>
    requires std::is_aggregate_v<std::decay_t<T>>
  constexpr size_t field_number_of =
      field_number_of_impl<std::decay_t<T>>::value;

  constexpr size_t MAX_FIELD_NUM = 20;

  template <typename T>
  concept is_std_array =
      requires {
        typename std::remove_cvref_t<T>::value_type;
        std::tuple_size<T>::value;
      }
      and std::is_same_v<
          T,
          std::array<typename T::value_type, std::tuple_size<T>::value>>;

  template <typename T>
  concept SimpleCodeableAggregate =
      std::is_aggregate_v<std::decay_t<T>>  //
      and (not std::is_array_v<T>)          //
      and (not is_std_array<T>)             //
      and (detail::field_number_of<T> <= MAX_FIELD_NUM);

  template <typename T, typename F>
    requires SimpleCodeableAggregate<T>
  auto &as_decomposed(T &&v, const F &f) {
    constexpr auto N = field_number_of<T>;
    // clang-format off
    if constexpr (N == 0) {
      return f();
    } else if constexpr (N == 1) {
      auto &[v1] = v;
      return f(v1);
    } else if constexpr (N == 2) {
      auto &[v1, v2] = v;
      return f(v1, v2);
    } else if constexpr (N == 3) {
      auto &[v1, v2, v3] = v;
      return f(v1, v2, v3);
    } else if constexpr (N == 4) {
      auto &[v1, v2, v3, v4] = v;
      return f(v1, v2, v3, v4);
    } else if constexpr (N == 5) {
      auto &[v1, v2, v3, v4, v5] = v;
     return f(v1, v2, v3, v4, v5);
    } else if constexpr (N == 6) {
      auto &[v1, v2, v3, v4, v5, v6] = v;
      return f(v1, v2, v3, v4, v5, v6);
    } else if constexpr (N == 7) {
      auto &[v1, v2, v3, v4, v5, v6, v7] = v;
      return f(v1, v2, v3, v4, v5, v6, v7);
    } else if constexpr (N == 8) {
      auto &[v1, v2, v3, v4, v5, v6, v7, v8] = v;
      return f(v1, v2, v3, v4, v5, v6, v7, v8);
    } else if constexpr (N == 9) {
      auto &[v1, v2, v3, v4, v5, v6, v7, v8, v9] = v;
      return f(v1, v2, v3, v4, v5, v6, v7, v8, v9);
    } else if constexpr (N == 10) {
      auto &[v1, v2, v3, v4, v5, v6, v7, v8, v9, v10] = v;
      return f(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
    } else if constexpr (N == 11) {
      auto &[v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11] = v;
      return f(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11);
    } else if constexpr (N == 12) {
      auto &[v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12] = v;
      return f(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12);
    } else if constexpr (N == 13) {
      auto &[v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13] = v;
      return f(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13);
    } else if constexpr (N == 14) {
      auto &[v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14] = v;
      return f(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14);
    } else if constexpr (N == 15) {
      auto &[v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15] = v;
      return f(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15);
    } else if constexpr (N == 16) {
      auto &[v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16] = v;
      return f(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16);
    } else if constexpr (N == 17) {
      auto &[v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17] = v;
      return f(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17);
    } else if constexpr (N == 18) {
      auto &[v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18] = v;
      return f(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18);
    } else if constexpr (N == 19) {
      auto &[v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19] = v;
      return f(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19);
    } else if constexpr (N == 20) {
      auto &[v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20] = v;
      return f(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20);
    } else if constexpr (N <= MAX_FIELD_NUM) {
      static_assert(!"Inconsistent value of MAX_FIELD_NUM");
    } else {
      static_assert(!"No code for cover aggregate with such big amount of fields");
    }
    // clang-format on
  }

}  // namespace scale::detail

namespace scale {

  class ScaleDecoderStream;
  class ScaleEncoderStream;

  template <typename T>
    requires detail::SimpleCodeableAggregate<T>
  ScaleEncoderStream &operator<<(ScaleEncoderStream &s, const T &v) {
    return detail::as_decomposed(  //
        v,
        [&](const auto &...args) -> ScaleEncoderStream & {
          return (s << ... << args);
        });
  }

  template <typename T>
    requires detail::SimpleCodeableAggregate<T>
  ScaleDecoderStream &operator>>(ScaleDecoderStream &s, T &v) {
    return detail::as_decomposed(  //
        v,
        [&](auto &...args) -> ScaleDecoderStream & {
          return (s >> ... >> args);
        });
  }

}  // namespace scale
