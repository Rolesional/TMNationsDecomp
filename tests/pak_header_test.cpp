#include "tmnf/pak/header.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <span>

namespace {

constexpr std::byte byte(const unsigned int value) { return static_cast<std::byte>(value); }

} // namespace

int main() {
  constexpr std::array fixture{
      byte('N'),  byte('a'),  byte('d'),  byte('e'),  byte('o'),  byte('P'),  byte('a'),
      byte('k'),  byte(0x03), byte(0x00), byte(0x00), byte(0x00), byte(0x01), byte(0x02),
      byte(0x03), byte(0x04), byte(0x05), byte(0x06), byte(0x07), byte(0x08), byte(0xA5),
  };

  const tmnf::pak::ParseResult parsed = tmnf::pak::parseHeader(std::span(fixture));
  if (!parsed || parsed.header.version != 3 ||
      parsed.header.initializationVector != 0x0807060504030201ULL ||
      parsed.header.encryptedHeaderOffset != 20) {
    std::cerr << "valid NadeoPak v3 fixture did not parse\n";
    return 1;
  }

  auto unsupported = fixture;
  unsupported[8] = byte(4);
  if (tmnf::pak::parseHeader(std::span(unsupported))) {
    std::cerr << "unsupported NadeoPak version was accepted\n";
    return 1;
  }

  const auto truncated = std::span(fixture).first<10>();
  if (tmnf::pak::parseHeader(truncated)) {
    std::cerr << "truncated NadeoPak header was accepted\n";
    return 1;
  }
  return 0;
}
