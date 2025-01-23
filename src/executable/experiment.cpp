#include <iostream>
#include <type_traits>

#include <../src_/TODO_scale/aggregate.hpp>
#include <../src_/jam/empty.hpp>
#include <../src_/jam/tagged.hpp>
#include <scale/scale.hpp>


namespace jam::test_vectors {
  class _1;
}


int main() {
  struct X {
    int e0 = 0;
    int e1 = 1;
    int e2 = 2;
    int e3 = 3;
    int e4 = 4;
    int e5 = 5;
    int e6 = 6;
    int e7 = 7;
    int e8 = 8;
    int e9 = 9;
    int e10 = 10;
    int e11 = 11;
    int e12 = 12;
    int e13 = 13;
    int e14 = 14;
    int e15 = 15;
    int e16 = 16;
    int e17 = 17;
    int e18 = 18;
    int e19 = 19;
  };

  X x;  // std::array<int, 12> x{1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2};

  auto z = scale::encode(x).value();

  //   std::vector<uint8_t> z;

  scale::ScaleDecoderStream s(z);

  X v;

  s >> v;

  auto c1 = std::is_aggregate_v<std::decay_t<X>>;
  auto c2 = not std::is_array_v<X>;
  auto c3 = not scale::detail::is_std_array<X>;
  auto c4 = scale::detail::field_number_of<X> <= 20;

  auto aa = not std::is_array_v<std::array<int, 3>>;
  auto na = std::is_array_v<X>;
  auto saa = scale::detail::is_std_array<std::array<int, 3>>;
  auto nsa = scale::detail::is_std_array<X>;


  s >> v;

  const jam::Tagged<jam::Empty, jam::test_vectors::_1> tagged_o;

  scale::ScaleEncoderStream es;

  es << tagged_o;

  jam::Tagged<jam::Empty, struct _> tagged;

  s >> tagged;

  // auto v = scale::decode<X>(z).value();

  struct Z {};

  std::cout << scale::detail::field_number_of<Z> << std::endl;
  std::cout << scale::detail::field_number_of<X> << std::endl;

  std::cout << scale::detail::field_number_of<jam::Empty> << std::endl;
  // std::cout << scale::detail::field_number_of<jam::Tagged<jam::Empty, struct
  // _>> << std::endl;

  std::cout << scale::detail::field_number_of<std::array<int, 123>>
            << std::endl;
}
