#include <morum/codec.hpp>

#include <iostream>

int main() {
  for (unsigned long i :
       {0ul, 1ul, 2ul, 3ul, 1ul << 28, 1ul << 29, 1ul << 25, 1ul << 63}) {
    morum::VectorStream stream;
    morum::encode(stream, i);
    std::cout << std::dec << i << " -> ";
    for (auto b : stream.data) {
      std::cout << std::hex << (int)b;
    }
    std::cout << "\n";
  }
}
