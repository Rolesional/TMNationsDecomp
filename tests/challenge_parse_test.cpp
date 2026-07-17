#include "tmnf/gbx/challenge.hpp"

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

void appendU32(std::vector<std::byte>& bytes, const std::uint32_t value) {
  for (std::size_t index = 0; index < 4; ++index) {
    bytes.push_back(static_cast<std::byte>((value >> (index * 8U)) & 0xffU));
  }
}

void appendString(std::vector<std::byte>& bytes, const std::string_view value) {
  appendU32(bytes, static_cast<std::uint32_t>(value.size()));
  bytes.insert(bytes.end(), reinterpret_cast<const std::byte*>(value.data()),
               reinterpret_cast<const std::byte*>(value.data() + value.size()));
}

void appendNewId(std::vector<std::byte>& bytes, const std::uint32_t tag,
                 const std::string_view value) {
  appendU32(bytes, tag);
  appendString(bytes, value);
}

void writeU32(std::vector<std::byte>& bytes, const std::size_t offset, const std::uint32_t value) {
  for (std::size_t index = 0; index < 4; ++index) {
    bytes[offset + index] = static_cast<std::byte>((value >> (index * 8U)) & 0xffU);
  }
}

struct Fixture final {
  std::vector<std::byte> body;
  std::size_t isLapRaceOffset = 0;
  std::size_t nbLapsOffset = 0;
  std::size_t secondBlockIdOffset = 0;
  std::size_t firstBlockFlagsOffset = 0;
};

Fixture makeFixture() {
  Fixture fixture;
  std::vector<std::byte>& bytes = fixture.body;
  appendU32(bytes, 0x0304300DU);
  appendU32(bytes, 3U);
  appendU32(bytes, std::numeric_limits<std::uint32_t>::max());
  appendU32(bytes, std::numeric_limits<std::uint32_t>::max());
  appendU32(bytes, std::numeric_limits<std::uint32_t>::max());

  appendU32(bytes, 0x03043017U);
  appendU32(bytes, 0x534B4950U);
  appendU32(bytes, 28U);
  appendU32(bytes, 2U);
  appendU32(bytes, 5U);
  appendU32(bytes, 11U);
  appendU32(bytes, 21U);
  appendU32(bytes, 12U);
  appendU32(bytes, 3U);
  appendU32(bytes, std::numeric_limits<std::uint32_t>::max());

  appendU32(bytes, 0x03043018U);
  appendU32(bytes, 0x534B4950U);
  appendU32(bytes, 8U);
  fixture.isLapRaceOffset = bytes.size();
  appendU32(bytes, 0U);
  fixture.nbLapsOffset = bytes.size();
  appendU32(bytes, 3U);

  appendU32(bytes, 0x0304301FU);
  appendNewId(bytes, 0x80000000U, "fixture-uid");
  appendNewId(bytes, 0x40000000U, "Stadium");
  appendNewId(bytes, 0x80000000U, "Nadeo");
  appendString(bytes, "Fixture Map");
  appendNewId(bytes, 0x80000000U, "Sunset");
  appendU32(bytes, 0x40000002U);
  appendU32(bytes, 0x80000003U);
  appendU32(bytes, 32U);
  appendU32(bytes, 32U);
  appendU32(bytes, 32U);
  appendU32(bytes, 0U);
  appendU32(bytes, 1U);
  appendU32(bytes, 2U);

  appendNewId(bytes, 0x80000000U, "StadiumCircuitBase");
  appendU8(bytes, 2U);
  appendU8(bytes, 30U);
  appendU8(bytes, 2U);
  appendU8(bytes, 9U);
  fixture.firstBlockFlagsOffset = bytes.size();
  appendU32(bytes, 4096U | (1U << 15U));
  appendU32(bytes, 0x80000003U);
  appendU32(bytes, 1U);
  appendU32(bytes, 0x03059000U);
  appendU32(bytes, 0x03059002U);
  appendString(bytes, "Fixture skin");
  appendU8(bytes, 2U);
  appendString(bytes, "Skins\\Any\\Fixture.zip");
  appendString(bytes, "");
  appendU8(bytes, 2U);
  appendString(bytes, "");
  appendU32(bytes, 0xFACADE01U);

  fixture.secondBlockIdOffset = bytes.size();
  appendU32(bytes, 0x80000005U);
  appendU8(bytes, 0U);
  appendU8(bytes, 30U);
  appendU8(bytes, 2U);
  appendU8(bytes, 10U);
  appendU32(bytes, 3U);
  appendU32(bytes, 0x03043021U);
  appendU32(bytes, 0xFACADE01U);
  return fixture;
}

} // namespace

int main() {
  const Fixture fixture = makeFixture();
  const tmnf::gbx::ChallengeParseResult parsed = tmnf::gbx::parseChallengeBody(fixture.body);
  if (!parsed) {
    std::cerr << parsed.error << '\n';
    return 1;
  }
  const tmnf::gbx::Challenge& challenge = parsed.challenge;
  if (challenge.mapInfo.id != "fixture-uid" || challenge.mapInfo.collection != "Stadium" ||
      challenge.mapInfo.author != "Nadeo" || challenge.mapName != "Fixture Map" ||
      challenge.decoration.id != "Sunset" || challenge.size[0] != 32 ||
      challenge.blockDataVersion != 1 || challenge.checkpoints.size() != 2 ||
      challenge.checkpoints[0] != std::array<std::int32_t, 3>{5, 11, 21} ||
      challenge.checkpoints[1] != std::array<std::int32_t, 3>{12, 3, -1} || challenge.isLapRace ||
      challenge.nbLaps != 3 || challenge.blocks.size() != 2 ||
      challenge.blocks[0].name != "StadiumCircuitBase" || challenge.blocks[0].direction != 2 ||
      challenge.blocks[0].coordinate[2] != 9 ||
      challenge.blocks[0].flags != (4096U | (1U << 15U)) || challenge.blocks[0].author != "Nadeo" ||
      !challenge.blocks[0].skin.has_value() || !challenge.blocks[0].skin->pack.has_value() ||
      challenge.blocks[0].skin->pack->filePath != "Skins\\Any\\Fixture.zip" ||
      challenge.blocks[1].name != "StadiumCircuitBase" || challenge.blocks[1].flags != 3 ||
      challenge.nextChunkId != 0x03043021U) {
    std::cerr << "challenge fixture decoded incorrectly\n";
    return 1;
  }

  std::vector<std::byte> a08LapRace = fixture.body;
  writeU32(a08LapRace, fixture.isLapRaceOffset, 1U);
  const tmnf::gbx::ChallengeParseResult parsedA08 = tmnf::gbx::parseChallengeBody(a08LapRace);
  if (!parsedA08 || !parsedA08.challenge.isLapRace || parsedA08.challenge.nbLaps != 3) {
    std::cerr << "A08 lap metadata decoded incorrectly\n";
    return 1;
  }

  std::vector<std::byte> e05LapRace = a08LapRace;
  writeU32(e05LapRace, fixture.nbLapsOffset, 60U);
  const tmnf::gbx::ChallengeParseResult parsedE05 = tmnf::gbx::parseChallengeBody(e05LapRace);
  if (!parsedE05 || !parsedE05.challenge.isLapRace || parsedE05.challenge.nbLaps != 60) {
    std::cerr << "E05 lap metadata decoded incorrectly\n";
    return 1;
  }

  std::vector<std::byte> invalidLapRace = fixture.body;
  writeU32(invalidLapRace, fixture.isLapRaceOffset, 2U);
  if (tmnf::gbx::parseChallengeBody(invalidLapRace)) {
    std::cerr << "invalid lap-race boolean was accepted\n";
    return 1;
  }

  std::vector<std::byte> unknownId = fixture.body;
  writeU32(unknownId, fixture.secondBlockIdOffset, 0x80000006U);
  if (tmnf::gbx::parseChallengeBody(unknownId)) {
    std::cerr << "unknown challenge ID dictionary entry was accepted\n";
    return 1;
  }
  std::vector<std::byte> missingSkin = fixture.body;
  writeU32(missingSkin, fixture.firstBlockFlagsOffset, 4096U);
  if (tmnf::gbx::parseChallengeBody(missingSkin)) {
    std::cerr << "orphaned challenge skin payload was accepted\n";
    return 1;
  }
  std::vector<std::byte> truncated = fixture.body;
  truncated.resize(truncated.size() - 9U);
  if (tmnf::gbx::parseChallengeBody(truncated)) {
    std::cerr << "truncated challenge body was accepted\n";
    return 1;
  }
  return 0;
}
