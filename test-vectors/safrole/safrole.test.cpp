/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <qtils/hex.hpp>

#include "safrole.hpp"
#include "vectors.hpp"

using jam::test_vectors_safrole::Vectors;
using _types = jam::test_vectors_safrole::tiny;

auto ok = true;

struct Indent {
  int indent = 0;
  Indent operator~() const {
    return Indent{indent + 1};
  }
};
template <>
struct fmt::formatter<Indent> {
  static constexpr auto parse(format_parse_context &ctx) {
    return ctx.begin();
  }
  auto format(const Indent &v, format_context &ctx) const {
    auto out = ctx.out();
    for (int i = 0; i < v.indent; ++i) {
      out = fmt::detail::write(out, "  ");
    }
    return out;
  }
};

#define DIFF_F(T) void diff(Indent indent, const T &v1, const T &v2)
DIFF_F(uint32_t);
DIFF_F(_types::CustomErrorCode);
DIFF_F(_types::EpochMark);
DIFF_F(_types::TicketEnvelope);
DIFF_F(_types::TicketBody);
DIFF_F(_types::OutputMarks);
DIFF_F(_types::Output);
DIFF_F(_types::ValidatorData);
DIFF_F(_types::TicketsOrKeys);
DIFF_F(_types::State);

enum class color { red = 31, green = 32 };
template <typename... T>
void _diff(color color, fmt::format_string<T...> f, T &&...a) {
  fmt::print("\x1b[0;{}m", (int)color);
  fmt::print(f, static_cast<T &&>(a)...);
  fmt::println("\x1b[0;0m");
}
template <typename... T>
void diff1(fmt::format_string<T...> f, T &&...a) {
  _diff(color::red, f, static_cast<T &&>(a)...);
}
template <typename... T>
void diff2(fmt::format_string<T...> f, T &&...a) {
  _diff(color::green, f, static_cast<T &&>(a)...);
}

#define DIFF_M(m) diff_m(indent, v1.m, v2.m, #m)
template <typename T>
void diff_m(Indent indent, const T &v1, const T &v2, std::string_view name) {
  if (v1 == v2) {
    return;
  }
  fmt::println("{}{}", indent, name);
  diff(~indent, v1, v2);
}
template <typename A, typename B>
void diff(Indent indent,
    const boost::variant<A, B> &v1,
    const boost::variant<A, B> &v2,
    const std::vector<std::string_view> &names) {
  if (v1 == v2) {
    return;
  }
  if (v1.which() != v2.which()) {
    fmt::println("{}{} != {}", indent, names[v1.which()], names[v2.which()]);
    return;
  }
  fmt::println("{}{}", indent, names[v1.which()]);
  if (v1.which() == 0) {
    diff(~indent, boost::get<A>(v1), boost::get<A>(v2));
  } else {
    diff(~indent, boost::get<B>(v1), boost::get<B>(v2));
  }
}
template <typename T>
void diff(
    Indent indent, const std::optional<T> &v1, const std::optional<T> &v2) {
  if (v1 == v2) {
    return;
  }
  std::string_view o[] = {"None", "Some"};
  auto i1 = v1 ? 1 : 0, i2 = v2 ? 1 : 0;
  if (i1 != i2) {
    fmt::println("{}{} != {}", indent, o[i1], o[i2]);
    return;
  }
  fmt::println("{}{}", indent, o[i1]);
  if (v1) {
    diff(~indent, *v1, *v2);
  }
}
template <typename T>
  requires(
      requires(T v) { std::span{v}; }
      and not requires(T v) { qtils::BytesIn{v}; })
void diff(Indent indent, const T &_v1, const T &_v2) {
  if (_v1 == _v2) {
    return;
  }
  std::span v1{_v1}, v2{_v2};
  if (v1.size() != v2.size()) {
    fmt::println("{}{} != {}", indent, v1.size(), v2.size());
    return;
  }
  fmt::println("{}{}", indent, v1.size());
  for (size_t i = 0; i < v1.size(); ++i) {
    if (v1[i] == v2[i]) {
      continue;
    }
    fmt::println("{}[{}]", indent, i);
    diff(~indent, v1[i], v2[i]);
  }
}
DIFF_F(qtils::BytesIn) {
  if (v1.size() == v2.size() && memcmp(v1.data(), v2.data(), v1.size()) == 0) {
    return;
  }
  diff1("{}- {:x}", indent, v1);
  diff2("{}+ {:x}", indent, v2);
}
DIFF_F(uint32_t) {
  fmt::println("{}{} != {}", indent, v1, v2);
}
DIFF_F(_types::CustomErrorCode) {
  std::string_view o[] = {
      "bad_slot",
      "unexpected_ticket",
      "bad_ticket_order",
      "bad_ticket_proof",
      "bad_ticket_attempt",
      "reserved",
      "duplicate_ticket",
  };
  fmt::println("{}{} != {}", indent, o[(int)v1], o[(int)v2]);
}

DIFF_F(_types::EpochMark) {
  DIFF_M(entropy);
  DIFF_M(validators);
}
DIFF_F(_types::TicketEnvelope) {
  DIFF_M(attempt);
  DIFF_M(signature);
}
DIFF_F(_types::TicketBody) {
  DIFF_M(id);
  DIFF_M(attempt);
}
DIFF_F(_types::OutputMarks) {
  DIFF_M(epoch_mark);
  DIFF_M(tickets_mark);
}
DIFF_F(_types::Output) {
  diff(indent, v1.v, v2.v, {"ok", "err"});
}
DIFF_F(_types::ValidatorData) {
  DIFF_M(bandersnatch);
  DIFF_M(ed25519);
  DIFF_M(bls);
  DIFF_M(metadata);
}
DIFF_F(_types::TicketsOrKeys) {
  diff(indent, v1.v, v2.v, {"tickets", "keys"});
}
DIFF_F(_types::State) {
  DIFF_M(tau);
  DIFF_M(eta);
  DIFF_M(lambda);
  DIFF_M(kappa);
  DIFF_M(gamma_k);
  DIFF_M(iota);
  DIFF_M(gamma_a);
  DIFF_M(gamma_s);
  DIFF_M(gamma_z);
}

/**
 * Check safrole state transition against test vectors.
 */
// template <bool full>
void test_transition() {
  constexpr auto full = false;
  Vectors<full> vectors;
  using types = decltype(vectors)::types;
  for (auto &path : vectors.paths) {
    auto s = path.native();
    fmt::println("{}.json", s.substr(0, s.size() - 6));
    auto testcase = vectors.read(path);
    auto [state, output] =
        jam::safrole::transition(testcase->pre_state, testcase->input);
    Indent indent{1};
    if (state != testcase->post_state) {
      ok = false;
      diff_m(indent, state, testcase->post_state, "state");
    }
    if (output != testcase->output) {
      ok = false;
      diff_m(indent, output, testcase->output, "output");
    }
  }
}

int main() {
  test_transition();
  // test_transition<false>();
  // test_transition<true>();
  if (ok) {
    fmt::println("ok");
  }
  return 0;
}
