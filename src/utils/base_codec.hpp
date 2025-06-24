/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <qtils/byte_vec.hpp>
#include <qtils/bytes.hpp>
#include <qtils/enum_error_code.hpp>
#include <qtils/outcome.hpp>

namespace jam::utils {
  enum class BaseCodecError {
    InvalidCharacter = 1,
    Overflow,
    OutputTooShort,
  };
}

OUTCOME_HPP_DECLARE_ERROR(jam::utils, BaseCodecError);

inline OUTCOME_CPP_DEFINE_CATEGORY(jam::utils, BaseCodecError, e) {
  using E = jam::utils::BaseCodecError;
  switch (e) {
    case E::InvalidCharacter:
      return "Invalid character in input";
    case E::Overflow:
      return "Integer overflow during decoding";
    case E::OutputTooShort:
      return "Decoded output too short";
  }
  __builtin_unreachable();
  return "Unknown BaseCodecError";
}

namespace jam::utils {
  template <size_t N, const char *Alphabet>
  class BaseCodec {
    static_assert(N > 1 && N <= 256, "Base must be in [2, 256]");

    static constexpr size_t DecodeTableSize = 256;
    static constexpr uint8_t Invalid = 0xFF;
    static constexpr int byte_size = 8;

    static consteval int calcDigitBits() {
      static_assert((N & (N - 1)) == 0, "N must be a power of 2");
      int r = 0;
      size_t v = N;
      while (v > 1) {
        v >>= 1;
        ++r;
      }
      return r;
    }

    static constexpr int digit_bits = calcDigitBits();

    static consteval auto makeDecodeTable()
        -> std::array<uint8_t, DecodeTableSize> {
      std::array<uint8_t, DecodeTableSize> table{};
      table.fill(Invalid);

      for (size_t i = 0; i < N; ++i) {
        auto ch = static_cast<uint8_t>(Alphabet[i]);
        table[ch] = static_cast<uint8_t>(i);
      }

      return table;
    }

    static constexpr std::array<uint8_t, DecodeTableSize> decodeTable =
        makeDecodeTable();


   public:
    static auto encode(qtils::BytesIn input) -> std::string {
      std::string out;
      size_t buffer = 0;
      int bits = 0;
      int bits_per_digit = static_cast<int>(std::ceil(std::log2(N)));

      for (auto b : input) {
        buffer = (buffer << 8) | static_cast<uint64_t>(b);
        bits += 8;

        while (bits >= bits_per_digit) {
          bits -= bits_per_digit;
          size_t idx = (buffer >> bits) & (N - 1);
          out += Alphabet[idx];
        }
      }

      if (bits > 0) {
        size_t idx = (buffer << (bits_per_digit - bits)) & (N - 1);
        out += Alphabet[idx];
      }

      return out;
    }

    static outcome::result<qtils::ByteVec> decode(std::string_view input) {
      qtils::ByteVec out;
      uint64_t buffer = 0;
      int bits = 0;


      for (char c : input) {
        auto uc = static_cast<unsigned char>(c);
        if (decodeTable[uc] == Invalid) {
          return BaseCodecError::InvalidCharacter;
        }

        uint8_t value = decodeTable[uc];

        // Буфер ограничен 64 битами: проверяем, что не переполняется после
        // добавления digit_bits
        if (bits + digit_bits > 64) {
          return BaseCodecError::Overflow;
        }

        buffer = (buffer << digit_bits) | value;
        bits += digit_bits;

        while (bits >= byte_size) {
          bits -= byte_size;
          auto byte = static_cast<uint8_t>((buffer >> bits) & 0xFF);
          out.push_back(byte);

          // Очищаем использованные биты
          buffer &= (1ULL << bits) - 1;
        }
      }

      return out;
    }

    static outcome::result<void> decode_into(
        std::string_view input, qtils::BytesOut output) {
      uint64_t buffer = 0;
      int bits = 0;
      size_t out_pos = 0;

      for (char c : input) {
        auto uc = static_cast<unsigned char>(c);
        if (decodeTable[uc] == Invalid) {
          return BaseCodecError::InvalidCharacter;
        }

        uint8_t value = decodeTable[uc];

        if (bits + digit_bits > 64) {
          return BaseCodecError::Overflow;
        }

        buffer = (buffer << digit_bits) | value;
        bits += digit_bits;

        while (bits >= byte_size) {
          if (out_pos >= output.size()) {
            return BaseCodecError::OutputTooShort;
          }

          bits -= byte_size;
          auto byte = static_cast<uint8_t>((buffer >> bits) & 0xFF);
          output[out_pos++] = byte;

          buffer &= (1ULL << bits) - 1;  // Clear used bits
        }
      }

      return outcome::success();
    }
  };


  template <size_t N, const char *A>
  constexpr std::array<uint8_t, 256> BaseCodec<N, A>::decodeTable;

}  // namespace jam::utils
