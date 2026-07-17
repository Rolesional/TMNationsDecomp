#include "tmnf/crypto/md5.hpp"

#include <array>
#include <bit>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

namespace tmnf::crypto {
namespace {

constexpr std::array<std::uint32_t, 64> kRoundConstants{
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391,
};

constexpr std::array<unsigned int, 64> kRotations{
    7,  12, 17, 22, 7,  12, 17, 22, 7,  12, 17, 22, 7,  12, 17, 22, 5,  9,  14, 20, 5,  9,
    14, 20, 5,  9,  14, 20, 5,  9,  14, 20, 4,  11, 16, 23, 4,  11, 16, 23, 4,  11, 16, 23,
    4,  11, 16, 23, 6,  10, 15, 21, 6,  10, 15, 21, 6,  10, 15, 21, 6,  10, 15, 21,
};

std::uint32_t readU32(const std::byte* const bytes) {
  std::uint32_t value = 0;
  for (std::size_t index = 0; index < 4; ++index) {
    value |= static_cast<std::uint32_t>(std::to_integer<unsigned char>(bytes[index]))
             << (index * 8U);
  }
  return value;
}

void writeU32(const std::uint32_t value, std::byte* const bytes) {
  for (std::size_t index = 0; index < 4; ++index) {
    bytes[index] = static_cast<std::byte>((value >> (index * 8U)) & 0xffU);
  }
}

} // namespace

Md5Digest md5(const std::span<const std::byte> bytes) {
  if (bytes.size() > std::numeric_limits<std::uint64_t>::max() / 8U) {
    throw std::length_error("MD5 input is too large");
  }

  std::vector<std::byte> padded(bytes.begin(), bytes.end());
  padded.push_back(std::byte{0x80});
  while (padded.size() % 64U != 56U) {
    padded.push_back(std::byte{0});
  }
  const std::uint64_t bitLength = static_cast<std::uint64_t>(bytes.size()) * 8U;
  for (std::size_t index = 0; index < 8; ++index) {
    padded.push_back(static_cast<std::byte>((bitLength >> (index * 8U)) & 0xffU));
  }

  std::uint32_t stateA = 0x67452301;
  std::uint32_t stateB = 0xefcdab89;
  std::uint32_t stateC = 0x98badcfe;
  std::uint32_t stateD = 0x10325476;

  for (std::size_t offset = 0; offset < padded.size(); offset += 64) {
    std::array<std::uint32_t, 16> words{};
    for (std::size_t index = 0; index < words.size(); ++index) {
      words[index] = readU32(padded.data() + offset + index * 4U);
    }

    std::uint32_t a = stateA;
    std::uint32_t b = stateB;
    std::uint32_t c = stateC;
    std::uint32_t d = stateD;
    for (std::size_t index = 0; index < 64; ++index) {
      std::uint32_t mixed = 0;
      std::size_t wordIndex = 0;
      if (index < 16) {
        mixed = (b & c) | (~b & d);
        wordIndex = index;
      } else if (index < 32) {
        mixed = (d & b) | (~d & c);
        wordIndex = (5U * index + 1U) % 16U;
      } else if (index < 48) {
        mixed = b ^ c ^ d;
        wordIndex = (3U * index + 5U) % 16U;
      } else {
        mixed = c ^ (b | ~d);
        wordIndex = (7U * index) % 16U;
      }

      const std::uint32_t previousD = d;
      d = c;
      c = b;
      b += std::rotl(a + mixed + kRoundConstants[index] + words[wordIndex], kRotations[index]);
      a = previousD;
    }

    stateA += a;
    stateB += b;
    stateC += c;
    stateD += d;
  }

  Md5Digest result{};
  writeU32(stateA, result.data());
  writeU32(stateB, result.data() + 4);
  writeU32(stateC, result.data() + 8);
  writeU32(stateD, result.data() + 12);
  return result;
}

Md5Digest md5(const std::string_view text) {
  return md5(std::span(reinterpret_cast<const std::byte*>(text.data()), text.size()));
}

std::string toHex(const Md5Digest& digest, const bool uppercase) {
  constexpr std::string_view kLower = "0123456789abcdef";
  constexpr std::string_view kUpper = "0123456789ABCDEF";
  const std::string_view alphabet = uppercase ? kUpper : kLower;
  std::string result(digest.size() * 2U, '0');
  for (std::size_t index = 0; index < digest.size(); ++index) {
    const unsigned char value = std::to_integer<unsigned char>(digest[index]);
    result[index * 2U] = alphabet[value >> 4U];
    result[index * 2U + 1U] = alphabet[value & 0x0fU];
  }
  return result;
}

} // namespace tmnf::crypto
