#include "tmnf/crypto/md5.hpp"
#include "tmnf/gbx/challenge.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace {

std::string jsonEscape(const std::string_view value) {
  std::string result;
  result.reserve(value.size());
  for (const unsigned char character : value) {
    switch (character) {
    case '"':
    case '\\':
      result.push_back('\\');
      result.push_back(static_cast<char>(character));
      break;
    case '\b':
      result += "\\b";
      break;
    case '\f':
      result += "\\f";
      break;
    case '\n':
      result += "\\n";
      break;
    case '\r':
      result += "\\r";
      break;
    case '\t':
      result += "\\t";
      break;
    default:
      if (character >= 0x20U) {
        result.push_back(static_cast<char>(character));
      }
      break;
    }
  }
  return result;
}

void appendU32(std::vector<std::byte>& bytes, const std::uint32_t value) {
  for (std::size_t index = 0; index < 4; ++index) {
    bytes.push_back(static_cast<std::byte>((value >> (index * 8U)) & 0xffU));
  }
}

std::string blockDigest(const std::vector<tmnf::gbx::ChallengeBlock>& blocks) {
  std::vector<std::byte> canonical;
  appendU32(canonical, static_cast<std::uint32_t>(blocks.size()));
  for (const tmnf::gbx::ChallengeBlock& block : blocks) {
    appendU32(canonical, static_cast<std::uint32_t>(block.name.size()));
    canonical.insert(canonical.end(), reinterpret_cast<const std::byte*>(block.name.data()),
                     reinterpret_cast<const std::byte*>(block.name.data() + block.name.size()));
    canonical.push_back(static_cast<std::byte>(block.direction));
    for (const std::uint8_t coordinate : block.coordinate) {
      canonical.push_back(static_cast<std::byte>(coordinate));
    }
    appendU32(canonical, block.flags);
  }
  return tmnf::crypto::toHex(tmnf::crypto::md5(canonical));
}

void writeOptionalTime(const std::optional<std::int32_t> value) {
  if (value.has_value()) {
    std::cout << *value;
  } else {
    std::cout << "null";
  }
}

} // namespace

int main(const int argc, const char* const* argv) {
  if (argc != 2) {
    std::cerr << "usage: challenge_inspect <file.Challenge.Gbx>\n";
    return 2;
  }
  const std::filesystem::path path(argv[1]);
  std::ifstream stream(path, std::ios::binary);
  if (!stream) {
    std::cerr << "unable to open: " << path << '\n';
    return 2;
  }
  const std::vector<char> raw((std::istreambuf_iterator<char>(stream)),
                              std::istreambuf_iterator<char>());
  const std::span bytes(reinterpret_cast<const std::byte*>(raw.data()), raw.size());
  const tmnf::gbx::ChallengeParseResult parsed = tmnf::gbx::parseChallenge(bytes);
  if (!parsed) {
    std::cerr << "unable to parse challenge: " << parsed.error << '\n';
    return 1;
  }
  const tmnf::gbx::Challenge& challenge = parsed.challenge;
  std::map<std::string, std::size_t, std::less<>> counts;
  std::map<std::tuple<std::string, bool, unsigned int, unsigned int>, std::size_t> variantCounts;
  std::size_t skinnedBlockCount = 0;
  for (const tmnf::gbx::ChallengeBlock& block : challenge.blocks) {
    ++counts[block.name];
    ++variantCounts[{block.name, (block.flags & (1U << 12U)) != 0U, block.flags & 63U,
                     (block.flags >> 6U) & 63U}];
    skinnedBlockCount += block.skin.has_value() ? 1U : 0U;
  }
  std::vector<std::pair<std::string, std::size_t>> orderedCounts(counts.begin(), counts.end());
  std::ranges::sort(orderedCounts, [](const auto& left, const auto& right) {
    return left.second != right.second ? left.second > right.second : left.first < right.first;
  });

  std::cout << "{\n"
            << "  \"mapUid\": \"" << jsonEscape(challenge.mapInfo.id) << "\",\n"
            << "  \"mapName\": \"" << jsonEscape(challenge.mapName) << "\",\n"
            << "  \"collection\": \"" << jsonEscape(challenge.mapInfo.collection) << "\",\n"
            << "  \"author\": \"" << jsonEscape(challenge.mapInfo.author) << "\",\n"
            << "  \"decoration\": \"" << jsonEscape(challenge.decoration.id) << "\",\n"
            << "  \"size\": [" << challenge.size[0] << ", " << challenge.size[1] << ", "
            << challenge.size[2] << "],\n"
            << "  \"needUnlock\": " << (challenge.needUnlock ? "true" : "false")
            << ",\n  \"medalTimes\": {\"bronze\": ";
  writeOptionalTime(challenge.medalTimes.bronzeMilliseconds);
  std::cout << ", \"silver\": ";
  writeOptionalTime(challenge.medalTimes.silverMilliseconds);
  std::cout << ", \"gold\": ";
  writeOptionalTime(challenge.medalTimes.goldMilliseconds);
  std::cout << ", \"author\": ";
  writeOptionalTime(challenge.medalTimes.authorMilliseconds);
  std::cout << "},\n"
            << "  \"cost\": " << challenge.cost << ",\n"
            << "  \"authorScore\": " << challenge.authorScore << ",\n"
            << "  \"blockDataVersion\": " << challenge.blockDataVersion << ",\n"
            << "  \"checkpointCount\": " << challenge.checkpoints.size() << ",\n"
            << "  \"isLapRace\": " << (challenge.isLapRace ? "true" : "false") << ",\n"
            << "  \"storedNbLaps\": " << challenge.nbLaps << ",\n"
            << "  \"blockCount\": " << challenge.blocks.size() << ",\n"
            << "  \"skinnedBlockCount\": " << skinnedBlockCount << ",\n"
            << "  \"uniqueBlockNameCount\": " << counts.size() << ",\n"
            << "  \"blockMd5\": \"" << blockDigest(challenge.blocks) << "\",\n"
            << "  \"blockDataChunkOffset\": " << challenge.blockDataChunkOffset << ",\n"
            << "  \"blockDataEndOffset\": " << challenge.blockDataEndOffset << ",\n"
            << "  \"nameCounts\": [\n";
  for (std::size_t index = 0; index < orderedCounts.size(); ++index) {
    const auto& [name, count] = orderedCounts[index];
    std::cout << "    {\"name\": \"" << jsonEscape(name) << "\", \"count\": " << count << "}"
              << (index + 1U == orderedCounts.size() ? "" : ",") << '\n';
  }
  std::cout << "  ],\n  \"variantCounts\": [\n";
  std::size_t variantIndex = 0;
  for (const auto& [key, count] : variantCounts) {
    const auto& [name, ground, variant, subVariant] = key;
    std::cout << "    {\"name\": \"" << jsonEscape(name)
              << "\", \"ground\": " << (ground ? "true" : "false") << ", \"variant\": " << variant
              << ", \"subVariant\": " << subVariant << ", \"count\": " << count << "}"
              << (++variantIndex == variantCounts.size() ? "" : ",") << '\n';
  }
  std::cout << "  ],\n  \"checkpoints\": [\n";
  for (std::size_t index = 0; index < challenge.checkpoints.size(); ++index) {
    const auto& checkpoint = challenge.checkpoints[index];
    std::cout << "    [" << checkpoint[0] << ", " << checkpoint[1] << ", " << checkpoint[2] << "]"
              << (index + 1U == challenge.checkpoints.size() ? "" : ",") << '\n';
  }
  std::cout << "  ],\n  \"waypointBlocks\": [\n";
  std::vector<const tmnf::gbx::ChallengeBlock*> waypoints;
  for (const tmnf::gbx::ChallengeBlock& block : challenge.blocks) {
    if (block.name.find("Start") != std::string::npos ||
        block.name.find("Finish") != std::string::npos ||
        block.name.find("Checkpoint") != std::string::npos) {
      waypoints.push_back(&block);
    }
  }
  for (std::size_t index = 0; index < waypoints.size(); ++index) {
    const tmnf::gbx::ChallengeBlock& block = *waypoints[index];
    std::cout << "    {\"name\": \"" << jsonEscape(block.name)
              << "\", \"direction\": " << static_cast<unsigned int>(block.direction)
              << ", \"coordinate\": [" << static_cast<unsigned int>(block.coordinate[0]) << ", "
              << static_cast<unsigned int>(block.coordinate[1]) << ", "
              << static_cast<unsigned int>(block.coordinate[2]) << "]}"
              << (index + 1U == waypoints.size() ? "" : ",") << '\n';
  }
  std::cout << "  ],\n  \"startNeighborhood\": [\n";
  const auto start = std::find_if(challenge.blocks.begin(), challenge.blocks.end(),
                                  [](const tmnf::gbx::ChallengeBlock& block) {
                                    return block.name.find("StartLine") != std::string::npos;
                                  });
  std::vector<const tmnf::gbx::ChallengeBlock*> startNeighborhood;
  if (start != challenge.blocks.end()) {
    for (const tmnf::gbx::ChallengeBlock& block : challenge.blocks) {
      const auto distance = [](const std::uint8_t left, const std::uint8_t right) {
        return left > right ? left - right : right - left;
      };
      if (distance(block.coordinate[0], start->coordinate[0]) <= 8U &&
          distance(block.coordinate[1], start->coordinate[1]) <= 3U &&
          distance(block.coordinate[2], start->coordinate[2]) <= 3U) {
        startNeighborhood.push_back(&block);
      }
    }
  }
  for (std::size_t index = 0; index < startNeighborhood.size(); ++index) {
    const tmnf::gbx::ChallengeBlock& block = *startNeighborhood[index];
    std::cout << "    {\"name\": \"" << jsonEscape(block.name)
              << "\", \"direction\": " << static_cast<unsigned int>(block.direction)
              << ", \"coordinate\": [" << static_cast<unsigned int>(block.coordinate[0]) << ", "
              << static_cast<unsigned int>(block.coordinate[1]) << ", "
              << static_cast<unsigned int>(block.coordinate[2]) << "], \"flags\": " << block.flags
              << "}" << (index + 1U == startNeighborhood.size() ? "" : ",") << '\n';
  }
  std::cout << "  ],\n  \"firstBlocks\": [\n";
  const std::size_t previewCount = std::min<std::size_t>(challenge.blocks.size(), 10U);
  for (std::size_t index = 0; index < previewCount; ++index) {
    const tmnf::gbx::ChallengeBlock& block = challenge.blocks[index];
    std::cout << "    {\"name\": \"" << jsonEscape(block.name)
              << "\", \"direction\": " << static_cast<unsigned int>(block.direction)
              << ", \"coordinate\": [" << static_cast<unsigned int>(block.coordinate[0]) << ", "
              << static_cast<unsigned int>(block.coordinate[1]) << ", "
              << static_cast<unsigned int>(block.coordinate[2]) << "], \"flags\": " << block.flags
              << "}" << (index + 1U == previewCount ? "" : ",") << '\n';
  }
  std::cout << "  ]\n}\n";
  return 0;
}
