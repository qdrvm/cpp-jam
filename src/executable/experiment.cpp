#include <iostream>

#include <qtils/tagged.hpp>
#include <scale/jam_scale.hpp>

struct S {
  explicit operator int() const {
    return 1;
  }
  S(int) {}
  S &operator+=(auto i) {
    return *this;
  }
  bool operator~() {
    return true;
  }
};

S &operator+=(S &s, auto i) {
  return s;
}

struct Tag {};

// void encode(qtils::is_tagged_v auto &&tagged, scale::ScaleEncoder auto &encoder) {
//   encode(untagged(tagged), encoder);
// }
// void decode(qtils::is_tagged_v auto &tagged, scale::ScaleDecoder auto &decoder) {
//   decode(untagged(tagged), decoder);
// }

int main() {
  qtils::Tagged<uint, Tag> x;
  qtils::Tagged<S, Tag> s(1);
  qtils::Tagged<std::vector<int>, Tag> v(1);
  qtils::Tagged<std::string, Tag> ts;
  int i;

  auto us = untagged(s);

  auto eq1x = 1 == x;
  auto eqx1 = x == 1;
  auto eqxx = x == x;
  x = 1;
  s = 1;
  x += 1;
  x += x;
  s += 1;
  s += x;
  auto &cx = x;
  s += cx;
  s += std::move(x);
  s += std::move(cx);
  s += s;
  // auto ss = x << 1;

  ++x;
  --x;
  auto ix = x++;
  auto dx = x--;

  scale::Encoder<scale::backend::ToBytes> es;
  es << x;
  es << v;

  scale::Decoder<scale::backend::FromBytes> ds(es.backend().to_vector());
  ds >> x;

  auto lx = x << 1;
  auto rx = x >> 1;

  auto &&w = !x;
  auto&&ww = s.operator~();
  auto &&wwww = ~x;
  auto z = ~i;
  auto zzzz = not x;

  return 0;
}
