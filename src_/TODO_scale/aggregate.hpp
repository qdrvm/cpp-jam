/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <type_traits>
#include <tuple>

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
      : public std::integral_constant<
            int,
            std::conditional_t<is_constructible_with_n_def_args_v<T, N + 1>,
                               field_number_of_impl<T, N + 1>,
                               std::integral_constant<int, N>>::value> {};

  template <typename T>
    requires std::is_aggregate_v<std::decay_t<T>>
  constexpr size_t field_number_of =
      field_number_of_impl<std::decay_t<T>>::value;

  template <typename T, typename F>
    requires std::is_aggregate_v<std::decay_t<T>>
  // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
  auto as_tie(T &&v, F &&f) {
    constexpr auto N = field_number_of<T>;
    // clang-format off
    if constexpr (N == 0) {
      return std::forward<F>(f)();
    } else if constexpr (N == 1) {
      auto &[v1] = v;
      return std::forward<F>(f)(v1);
    } else if constexpr (N == 2) {
      auto &[v1, v2] = v;
      return std::forward<F>(f)(v1, v2);
    } else if constexpr (N == 3) {
      auto &[v1, v2, v3] = v;
      return std::forward<F>(f)(v1, v2, v3);
    } else if constexpr (N == 4) {
      auto &[v1, v2, v3, v4] = v;
      return std::forward<F>(f)(v1, v2, v3, v4);
    } else if constexpr (N == 5) {
      auto &[v1, v2, v3, v4, v5] = v;
     return std::forward<F>(f)(v1, v2, v3, v4, v5);
    } else if constexpr (N == 6) {
      auto &[v1, v2, v3, v4, v5, v6] = v;
      return std::forward<F>(f)(v1, v2, v3, v4, v5, v6);
    } else if constexpr (N == 7) {
      auto &[v1, v2, v3, v4, v5, v6, v7] = v;
      return std::forward<F>(f)(v1, v2, v3, v4, v5, v6, v7);
    } else if constexpr (N == 8) {
      auto &[v1, v2, v3, v4, v5, v6, v7, v8] = v;
      return std::forward<F>(f)(v1, v2, v3, v4, v5, v6, v7, v8);
    } else if constexpr (N == 9) {
      auto &[v1, v2, v3, v4, v5, v6, v7, v8, v9] = v;
      return std::forward<F>(f)(v1, v2, v3, v4, v5, v6, v7, v8, v9);
    } else if constexpr (N == 10) {
      auto &[v1, v2, v3, v4, v5, v6, v7, v8, v9, v10] = v;
      return std::forward<F>(f)(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
    } else if constexpr (N == 11) {
      auto &[v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11] = v;
      return std::forward<F>(f)(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11);
    } else if constexpr (N == 12) {
      auto &[v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12] = v;
      return std::forward<F>(f)(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12);
    } else if constexpr (N == 13) {
      auto &[v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13] = v;
      return std::forward<F>(f)(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13);
    } else if constexpr (N == 14) {
      auto &[v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14] = v;
      return std::forward<F>(f)(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14);
    } else if constexpr (N == 15) {
      auto &[v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15] = v;
      return std::forward<F>(f)(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15);
    } else if constexpr (N == 16) {
      auto &[v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16] = v;
      return std::forward<F>(f)(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16);
    } else if constexpr (N == 17) {
      auto &[v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17] = v;
      return std::forward<F>(f)(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17);
    } else if constexpr (N == 18) {
      auto &[v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18] = v;
      return std::forward<F>(f)(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18);
    } else if constexpr (N == 19) {
      auto &[v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19] = v;
      return std::forward<F>(f)(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19);
    } else if constexpr (N == 20) {
      auto &[v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20] = v;
      return std::forward<F>(f)(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20);
    } else {
      // generate code for bigger tuples
      BOOST_STATIC_ASSERT_MSG(N > 15, "No code for such big tuple");
    }
    // clang-format on
  }

}  // namespace scale::detail

namespace scale {
  class ScaleEncoderStream;
  class ScaleDecoderStream;

  template <typename T,
            typename = decltype(detail::as_tie(std::declval<T>(),
                                               [](const auto &...) {}))>
    requires std::is_aggregate_v<std::decay_t<T>>
  ScaleEncoderStream &operator<<(ScaleEncoderStream &s, const T &v) {
    detail::as_tie(v, [&](const auto &...args) { return (s << ... << args); });
    return s;
  }

  template <typename T,
            typename = decltype(detail::as_tie(std::declval<T>(),
                                               [](auto &...) {}))>
    requires std::is_aggregate_v<std::decay_t<T>>
  ScaleDecoderStream &operator>>(ScaleDecoderStream &s, T &v) {
    detail::as_tie(v, [&](auto &...args) { return (s >> ... >> args); });
    return s;
  }

}  // namespace scale
