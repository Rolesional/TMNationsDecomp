#include "tmnf/gbx/class_registry.hpp"
#include "tmnf/gbx/header.hpp"

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
      byte('G'),  byte('B'),  byte('X'),  byte(0x06), byte(0x00), byte('B'),
      byte('U'),  byte('U'),  byte('R'),  byte(0x00), byte(0x10), byte(0x00),
      byte(0x01), byte(0x00), byte(0x00), byte(0x00), byte(0x00),
  };

  const tmnf::gbx::ParseResult parsed = tmnf::gbx::parseHeader(std::span(fixture));
  if (!parsed) {
    std::cerr << parsed.error << '\n';
    return 1;
  }
  if (parsed.header.version != 6 || parsed.header.classId != 0x01001000U ||
      parsed.header.bodyOffset != fixture.size() || !parsed.header.userDataChunks.empty()) {
    std::cerr << "parsed fixture did not match expected header\n";
    return 1;
  }
  if (tmnf::gbx::classRegistry().size() != 761 || tmnf::gbx::className(0x01001000U) != "CMwNod" ||
      tmnf::gbx::className(0x03043000U) != "CGameCtnChallenge" ||
      tmnf::gbx::className(0x03093000U) != "CGameCtnReplayRecord" ||
      tmnf::gbx::classParentId(0x03043000U) != 0x01001000U ||
      tmnf::gbx::dummyWriteSeedClassId(0x0304F000U) != 0x24005000U ||
      tmnf::gbx::className(0xffffffffU).has_value()) {
    std::cerr << "recovered class registry did not match expected entries\n";
    return 1;
  }
  return 0;
}
