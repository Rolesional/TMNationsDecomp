#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace tmnf::gbx {

struct ChallengeIdent final {
  std::string id;
  std::string collection;
  std::string author;
};

struct ChallengePackDesc final {
  std::string filePath;
  std::string locatorUrl;
};

struct ChallengeBlockSkin final {
  std::string text;
  std::optional<ChallengePackDesc> pack;
  std::optional<ChallengePackDesc> parentPack;
  std::optional<ChallengePackDesc> foregroundPack;
};

struct ChallengeBlock final {
  std::string name;
  std::uint8_t direction = 0;
  std::array<std::uint8_t, 3> coordinate{};
  std::uint32_t flags = 0;
  std::string author;
  std::optional<ChallengeBlockSkin> skin;
};

struct ChallengeMedalTimes final {
  std::optional<std::int32_t> bronzeMilliseconds;
  std::optional<std::int32_t> silverMilliseconds;
  std::optional<std::int32_t> goldMilliseconds;
  std::optional<std::int32_t> authorMilliseconds;
};

struct ChallengeHeaderMetadata final {
  ChallengeIdent mapInfo;
  std::string mapName;
  ChallengeMedalTimes medalTimes;
  bool needUnlock = false;
  bool isLapRace = false;
  bool hasClones = false;
  std::int32_t cost = 0;
  std::int32_t playMode = 0;
  std::int32_t authorScore = 0;
  std::int32_t editorMode = 0;
  std::uint8_t mapKind = 0;
};

struct ChallengeHeaderParseResult final {
  ChallengeHeaderMetadata metadata;
  std::string error;

  [[nodiscard]] explicit operator bool() const noexcept { return error.empty(); }
};

struct Challenge final {
  ChallengeIdent mapInfo;
  std::string mapName;
  ChallengeIdent decoration;
  std::array<std::int32_t, 3> size{};
  bool needUnlock = false;
  ChallengeMedalTimes medalTimes;
  std::int32_t cost = 0;
  std::int32_t authorScore = 0;
  std::int32_t blockDataVersion = 0;
  std::vector<std::array<std::int32_t, 3>> checkpoints;
  bool isLapRace = false;
  // Serialized chunk value; TMF ignores it when isLapRace is false.
  std::int32_t nbLaps = 0;
  std::vector<ChallengeBlock> blocks;
  std::size_t blockDataChunkOffset = 0;
  std::size_t blockDataEndOffset = 0;
  std::uint32_t nextChunkId = 0;
};

struct ChallengeParseResult final {
  Challenge challenge;
  std::string error;

  [[nodiscard]] explicit operator bool() const noexcept { return error.empty(); }
};

// Parses an already-uncompressed CGameCtnChallenge body. The TMF block-data
// archive is version 1; version 0 is accepted for older maps as well.
[[nodiscard]] ChallengeParseResult parseChallengeBody(std::span<const std::byte> bodyBytes);

// Probes a loose GBX document, decodes its body, and parses its challenge block
// placements.
[[nodiscard]] ChallengeParseResult parseChallenge(std::span<const std::byte> documentBytes);

// Reads the lightweight challenge user-data chunks used by campaign selectors.
[[nodiscard]] ChallengeHeaderParseResult
parseChallengeHeaderMetadata(std::span<const std::byte> documentBytes);

} // namespace tmnf::gbx
