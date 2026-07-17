#include "tmnf/gbx/block_info.hpp"
#include "tmnf/gbx/span_input.hpp"

#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string_view>
#include <vector>

namespace {

void appendU8(std::vector<std::byte>& bytes, const std::uint8_t value) {
  bytes.push_back(static_cast<std::byte>(value));
}

void appendU16(std::vector<std::byte>& bytes, const std::uint16_t value) {
  appendU8(bytes, static_cast<std::uint8_t>(value));
  appendU8(bytes, static_cast<std::uint8_t>(value >> 8U));
}

void appendU32(std::vector<std::byte>& bytes, const std::uint32_t value) {
  for (std::size_t index = 0; index < 4; ++index) {
    appendU8(bytes, static_cast<std::uint8_t>(value >> (index * 8U)));
  }
}

void appendFloat(std::vector<std::byte>& bytes, const float value) {
  appendU32(bytes, std::bit_cast<std::uint32_t>(value));
}

void appendString(std::vector<std::byte>& bytes, const std::string_view value) {
  appendU32(bytes, static_cast<std::uint32_t>(value.size()));
  bytes.insert(bytes.end(), reinterpret_cast<const std::byte*>(value.data()),
               reinterpret_cast<const std::byte*>(value.data() + value.size()));
}

void appendIso4(std::vector<std::byte>& bytes, const float x, const float y, const float z) {
  constexpr std::array rotation{1.0F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F, 0.0F, 1.0F};
  for (const float value : rotation) {
    appendFloat(bytes, value);
  }
  appendFloat(bytes, x);
  appendFloat(bytes, y);
  appendFloat(bytes, z);
}

struct Fixture final {
  std::vector<std::byte> bytes;
  std::size_t waypointOffset = 0;
  std::size_t noRespawnOffset = 0;
};

Fixture makeFixture() {
  Fixture fixture;
  std::vector<std::byte>& bytes = fixture.bytes;
  appendU8(bytes, 'G');
  appendU8(bytes, 'B');
  appendU8(bytes, 'X');
  appendU16(bytes, 6U);
  appendU8(bytes, 'B');
  appendU8(bytes, 'U');
  appendU8(bytes, 'U');
  appendU8(bytes, 'R');
  appendU32(bytes, 0x03051000U);
  appendU32(bytes, 0U);
  appendU32(bytes, 1U);
  appendU32(bytes, 0U);

  appendU32(bytes, 0x0301A006U);
  appendU32(bytes, 0U);
  appendU32(bytes, 0x0301A007U);
  for (int index = 0; index < 6; ++index) {
    appendU32(bytes, 0U);
  }
  appendU32(bytes, 0x0301A009U);
  appendString(bytes, "RoadMain/Race/");
  appendU32(bytes, 0U);
  appendU32(bytes, 3U);
  appendU32(bytes, std::numeric_limits<std::uint32_t>::max());
  appendU32(bytes, 0x0301A00AU);
  appendU32(bytes, std::numeric_limits<std::uint32_t>::max());
  appendU32(bytes, 0x0301A00BU);
  appendU32(bytes, std::numeric_limits<std::uint32_t>::max());
  appendU32(bytes, std::numeric_limits<std::uint32_t>::max());
  appendU32(bytes, std::numeric_limits<std::uint32_t>::max());

  appendU32(bytes, 0x0304E005U);
  appendU32(bytes, std::numeric_limits<std::uint32_t>::max());
  for (int index = 0; index < 6; ++index) {
    appendU32(bytes, 0U);
  }
  appendU32(bytes, std::numeric_limits<std::uint32_t>::max());
  appendU32(bytes, 0U);
  appendU32(bytes, 0U);
  appendU32(bytes, 0U);
  appendU32(bytes, 0U);
  appendU8(bytes, 0U);
  appendU32(bytes, 0U);
  appendU16(bytes, 0U);
  appendU16(bytes, 0U);
  appendU32(bytes, 0x0304E009U);
  appendU32(bytes, 0U);
  appendU32(bytes, 0x0304E00CU);
  appendIso4(bytes, 16.0F, 2.21F, 11.2F);
  appendIso4(bytes, 16.0F, 2.21F, 11.2F);
  appendU32(bytes, 0x0304E00DU);
  appendU32(bytes, 0U);
  appendU32(bytes, 0x0304E00EU);
  fixture.waypointOffset = bytes.size();
  appendU32(bytes, 0U);
  appendU32(bytes, 5U);
  appendU32(bytes, 0x0A011000U);
  appendU32(bytes, 0xFACADE01U);
  appendU32(bytes, std::numeric_limits<std::uint32_t>::max());
  appendU32(bytes, std::numeric_limits<std::uint32_t>::max());
  appendU32(bytes, 0x0304E00FU);
  fixture.noRespawnOffset = bytes.size();
  appendU32(bytes, 0U);
  return fixture;
}

void writeU32(std::vector<std::byte>& bytes, const std::size_t offset, const std::uint32_t value) {
  for (std::size_t index = 0; index < 4; ++index) {
    bytes[offset + index] = static_cast<std::byte>(value >> (index * 8U));
  }
}

} // namespace

int main() {
  const Fixture fixture = makeFixture();
  tmnf::gbx::SpanInput input(fixture.bytes);
  const tmnf::gbx::BlockInfoParseResult parsed = tmnf::gbx::parseBlockInfo(input);
  if (!parsed) {
    std::cerr << parsed.error << '\n';
    return 1;
  }
  if (parsed.blockInfo.waypointType != tmnf::gbx::BlockWaypointType::Start ||
      !parsed.blockInfo.spawnGround.has_value() || !parsed.blockInfo.spawnAir.has_value() ||
      parsed.blockInfo.noRespawn || std::abs((*parsed.blockInfo.spawnAir)[9] - 16.0F) > 1.0e-6F ||
      std::abs((*parsed.blockInfo.spawnAir)[10] - 2.21F) > 1.0e-6F ||
      std::abs((*parsed.blockInfo.spawnAir)[11] - 11.2F) > 1.0e-6F) {
    std::cerr << "BlockInfo fixture metadata decoded incorrectly\n";
    return 1;
  }

  std::vector<std::byte> noRespawn = fixture.bytes;
  writeU32(noRespawn, fixture.noRespawnOffset, 1U);
  tmnf::gbx::SpanInput noRespawnInput(noRespawn);
  const tmnf::gbx::BlockInfoParseResult noRespawnParsed = tmnf::gbx::parseBlockInfo(noRespawnInput);
  if (!noRespawnParsed || !noRespawnParsed.blockInfo.noRespawn) {
    std::cerr << "BlockInfo NoRespawn flag decoded incorrectly\n";
    return 1;
  }

  std::vector<std::byte> invalidWaypoint = fixture.bytes;
  writeU32(invalidWaypoint, fixture.waypointOffset, 99U);
  tmnf::gbx::SpanInput invalidInput(invalidWaypoint);
  if (tmnf::gbx::parseBlockInfo(invalidInput)) {
    std::cerr << "invalid BlockInfo waypoint was accepted\n";
    return 1;
  }

  std::vector<std::byte> truncated = fixture.bytes;
  truncated.resize(fixture.waypointOffset - 20U);
  tmnf::gbx::SpanInput truncatedInput(truncated);
  if (tmnf::gbx::parseBlockInfo(truncatedInput)) {
    std::cerr << "truncated BlockInfo spawn matrix was accepted\n";
    return 1;
  }
  return 0;
}
