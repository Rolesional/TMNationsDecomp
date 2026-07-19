#include "tmnf/content/block_assets.hpp"
#include "tmnf/gbx/challenge.hpp"
#include "tmnf/pak/vfs.hpp"

#include <array>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

bool readFile(const std::filesystem::path& path, std::vector<std::byte>& output) {
  std::ifstream stream(path, std::ios::binary);
  if (!stream) {
    return false;
  }
  stream.seekg(0, std::ios::end);
  const std::streamoff size = stream.tellg();
  if (size < 0) {
    return false;
  }
  output.resize(static_cast<std::size_t>(size));
  stream.seekg(0, std::ios::beg);
  if (!output.empty()) {
    stream.read(reinterpret_cast<char*>(output.data()), size);
  }
  return static_cast<bool>(stream);
}

std::string jsonEscape(const std::string_view value) {
  std::string result;
  result.reserve(value.size());
  for (const unsigned char character : value) {
    if (character == '"' || character == '\\') {
      result.push_back('\\');
      result.push_back(static_cast<char>(character));
    } else if (character >= 0x20U) {
      result.push_back(static_cast<char>(character));
    }
  }
  return result;
}

std::string_view roleName(const tmnf::content::BlockSolidRole role) {
  switch (role) {
  case tmnf::content::BlockSolidRole::Ground:
    return "ground";
  case tmnf::content::BlockSolidRole::Air:
    return "air";
  default:
    return "any";
  }
}

std::string_view waypointName(const tmnf::gbx::BlockWaypointType type) {
  switch (type) {
  case tmnf::gbx::BlockWaypointType::Start:
    return "start";
  case tmnf::gbx::BlockWaypointType::Finish:
    return "finish";
  case tmnf::gbx::BlockWaypointType::Checkpoint:
    return "checkpoint";
  case tmnf::gbx::BlockWaypointType::StartFinish:
    return "start-finish";
  case tmnf::gbx::BlockWaypointType::Dispenser:
    return "dispenser";
  default:
    return "none";
  }
}

void writeTransform(const std::optional<std::array<float, 12>>& transform) {
  if (!transform.has_value()) {
    std::cout << "null";
    return;
  }
  std::cout << '[';
  for (std::size_t index = 0; index < transform->size(); ++index) {
    std::cout << (*transform)[index] << (index + 1U == transform->size() ? "" : ", ");
  }
  std::cout << ']';
}

} // namespace

int main(const int argc, const char* const* argv) {
  if (argc != 4) {
    std::cerr << "usage: archive_block_assets <Stadium.pak> <packlist.dat> <Challenge.Gbx>\n";
    return 2;
  }
  std::vector<std::byte> archiveBytes;
  std::vector<std::byte> packListBytes;
  std::vector<std::byte> challengeBytes;
  if (!readFile(argv[1], archiveBytes) || !readFile(argv[2], packListBytes) ||
      !readFile(argv[3], challengeBytes)) {
    std::cerr << "unable to read archive, packlist, or challenge\n";
    return 2;
  }
  tmnf::pak::VirtualFileSystem vfs;
  const tmnf::pak::VfsOperationResult initialized = vfs.initialize(packListBytes);
  if (!initialized) {
    std::cerr << initialized.error << '\n';
    return 1;
  }
  const tmnf::pak::VfsOperationResult mounted = vfs.mount("Stadium", std::move(archiveBytes));
  if (!mounted) {
    std::cerr << mounted.error << '\n';
    return 1;
  }
  const tmnf::gbx::ChallengeParseResult parsed = tmnf::gbx::parseChallenge(challengeBytes);
  if (!parsed) {
    std::cerr << parsed.error << '\n';
    return 1;
  }
  std::map<std::string, std::size_t, std::less<>> counts;
  for (const tmnf::gbx::ChallengeBlock& block : parsed.challenge.blocks) {
    ++counts[block.name];
  }
  std::vector<tmnf::content::BlockAssetResolveResult> graphs;
  graphs.reserve(counts.size());
  std::size_t resolvedCount = 0;
  std::size_t blockNamesWithSolids = 0;
  std::size_t solidReferenceCount = 0;
  std::size_t blockNamesWithTriggers = 0;
  std::size_t triggerSolidReferenceCount = 0;
  std::size_t parsedMetadataCount = 0;
  for (const auto& [name, count] : counts) {
    (void)count;
    tmnf::content::BlockAssetResolveResult graph =
        tmnf::content::resolveBlockAssets(parsed.challenge.mapInfo.collection, name, vfs);
    resolvedCount += graph ? 1U : 0U;
    if (graph) {
      blockNamesWithSolids += graph.graph.solids.empty() ? 0U : 1U;
      solidReferenceCount += graph.graph.solids.size();
      blockNamesWithTriggers += graph.graph.triggerSolids.empty() ? 0U : 1U;
      triggerSolidReferenceCount += graph.graph.triggerSolids.size();
      parsedMetadataCount += graph.graph.metadata.has_value() ? 1U : 0U;
    }
    graphs.push_back(std::move(graph));
  }

  std::cout << "{\n"
            << "  \"mapName\": \"" << jsonEscape(parsed.challenge.mapName) << "\",\n"
            << "  \"uniqueBlockNameCount\": " << counts.size() << ",\n"
            << "  \"resolvedBlockInfoCount\": " << resolvedCount << ",\n"
            << "  \"blockNamesWithSolids\": " << blockNamesWithSolids << ",\n"
            << "  \"solidReferenceCount\": " << solidReferenceCount << ",\n"
            << "  \"blockNamesWithTriggers\": " << blockNamesWithTriggers << ",\n"
            << "  \"triggerSolidReferenceCount\": " << triggerSolidReferenceCount << ",\n"
            << "  \"parsedMetadataCount\": " << parsedMetadataCount << ",\n"
            << "  \"blocks\": [\n";
  std::size_t graphIndex = 0;
  for (const auto& [name, count] : counts) {
    const tmnf::content::BlockAssetResolveResult& resolved = graphs[graphIndex++];
    std::cout << "    {\"name\": \"" << jsonEscape(name) << "\", \"placements\": " << count;
    if (!resolved) {
      std::cout << ", \"error\": \"" << jsonEscape(resolved.error) << "\"}";
    } else {
      const tmnf::content::BlockAssetGraph& graph = resolved.graph;
      std::cout << ", \"blockInfo\": \"" << jsonEscape(graph.blockInfoPath)
                << "\", \"probedNodes\": " << graph.probedNodeCount
                << ", \"groundFootprint\": [" << graph.groundFootprint[0] << ", "
                << graph.groundFootprint[1] << "], \"airFootprint\": ["
                << graph.airFootprint[0] << ", " << graph.airFootprint[1]
                << "], \"metadata\": ";
      if (!graph.metadata.has_value()) {
        std::cout << "null";
      } else {
        std::cout << "{\"waypoint\": \"" << waypointName(graph.metadata->waypointType)
                  << "\", \"spawnGround\": ";
        writeTransform(graph.metadata->spawnGround);
        std::cout << ", \"spawnAir\": ";
        writeTransform(graph.metadata->spawnAir);
        std::cout << ", \"noRespawn\": " << (graph.metadata->noRespawn ? "true" : "false");
        std::cout << '}';
      }
      std::cout << ", \"solids\": [";
      for (std::size_t index = 0; index < graph.solids.size(); ++index) {
        const tmnf::content::BlockSolidAsset& solid = graph.solids[index];
        std::cout << "{\"path\": \"" << jsonEscape(solid.logicalPath) << "\", \"role\": \""
                  << roleName(solid.role) << "\"}"
                  << (index + 1U == graph.solids.size() ? "" : ", ");
      }
      std::cout << "], \"triggerSolids\": [";
      for (std::size_t index = 0; index < graph.triggerSolids.size(); ++index) {
        const tmnf::content::BlockSolidAsset& solid = graph.triggerSolids[index];
        std::cout << "{\"path\": \"" << jsonEscape(solid.logicalPath) << "\", \"role\": \""
                  << roleName(solid.role) << "\"}"
                  << (index + 1U == graph.triggerSolids.size() ? "" : ", ");
      }
      std::cout << "], \"unresolved\": [";
      for (std::size_t index = 0; index < graph.unresolvedReferences.size(); ++index) {
        std::cout << "\"" << jsonEscape(graph.unresolvedReferences[index]) << "\""
                  << (index + 1U == graph.unresolvedReferences.size() ? "" : ", ");
      }
      std::cout << "]}";
    }
    std::cout << (graphIndex == graphs.size() ? "" : ",") << '\n';
  }
  std::cout << "  ]\n}\n";
  return resolvedCount == counts.size() ? 0 : 1;
}
