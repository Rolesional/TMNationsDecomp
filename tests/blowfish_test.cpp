#include "tmnf/crypto/blowfish.hpp"

#include <array>
#include <cstddef>
#include <iostream>

int main() {
  constexpr std::array<std::byte, 8> key{};
  std::array<std::byte, 8> block{};
  const tmnf::crypto::BlowfishLittleEndian blowfish(key);
  blowfish.encryptBlock(block);

  constexpr std::array expected{
      std::byte{0x45}, std::byte{0x97}, std::byte{0xF9}, std::byte{0x4E},
      std::byte{0x78}, std::byte{0xDD}, std::byte{0x98}, std::byte{0x61},
  };
  if (block != expected) {
    std::cerr << "little-endian Blowfish did not match the standard zero vector\n";
    return 1;
  }
  blowfish.decryptBlock(block);
  if (block != std::array<std::byte, 8>{}) {
    std::cerr << "little-endian Blowfish did not decrypt its test vector\n";
    return 1;
  }
  return 0;
}
