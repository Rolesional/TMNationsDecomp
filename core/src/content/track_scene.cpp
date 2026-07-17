#include "tmnf/content/track_scene.hpp"

#include "tmnf/content/block_assets.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <optional>
#include <span>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace tmnf::content {
namespace {

constexpr std::uint32_t kChallengeBlockGroundFlag = 1U << 12U;

TrackSceneBuildResult failed(std::string message) {
  TrackSceneBuildResult result;
  result.error = std::move(message);
  return result;
}

std::string lowercase(const std::string_view value) {
  std::string result(value);
  std::ranges::transform(result, result.begin(), [](const unsigned char character) {
    return static_cast<char>(std::tolower(character));
  });
  return result;
}

std::string_view fileName(const std::string_view path) {
  const std::size_t separator = path.find_last_of("\\/");
  return separator == std::string_view::npos ? path : path.substr(separator + 1U);
}

bool isProceduralRoad(const std::string_view blockName) {
  return blockName == "StadiumTubeRoad" || blockName == "StadiumTubeRoadCross" ||
         blockName == "StadiumControlRoadGlass";
}

bool expectedWaypointMetadata(const std::string_view blockName) {
  return blockName.find("StartLine") != std::string_view::npos ||
         blockName.find("FinishLine") != std::string_view::npos ||
         blockName.find("Checkpoint") != std::string_view::npos;
}

const BlockSolidAsset* selectSolid(const std::span<const BlockSolidAsset> solids,
                                   const std::string_view blockName,
                                   const gbx::ChallengeBlock& block) {
  if (solids.empty()) {
    return nullptr;
  }
  const bool ground = (block.flags & kChallengeBlockGroundFlag) != 0U;
  const BlockSolidRole wantedRole = ground ? BlockSolidRole::Ground : BlockSolidRole::Air;
  std::vector<const BlockSolidAsset*> candidates;
  for (const BlockSolidAsset& solid : solids) {
    if (solid.role == wantedRole || solid.role == BlockSolidRole::Any) {
      candidates.push_back(&solid);
    }
  }
  if (candidates.empty() && solids.size() == 1U) {
    return &solids.front();
  }
  if (candidates.empty()) {
    return nullptr;
  }
  if (isProceduralRoad(blockName)) {
    constexpr std::array<std::string_view, 6> kFrontierNames{"base",     "deadend", "corner",
                                                             "straight", "tshaped", "cross"};
    const std::uint32_t variant = block.flags & 63U;
    const std::string_view topology =
        blockName == "StadiumTubeRoadCross"
            ? std::string_view("cross")
            : kFrontierNames[std::min<std::uint32_t>(variant, kFrontierNames.size() - 1U)];
    for (const BlockSolidAsset* const candidate : candidates) {
      const std::string name = lowercase(fileName(candidate->logicalPath));
      if (name.starts_with(topology)) {
        return candidate;
      }
    }
  }
  const auto exactRole = std::ranges::find_if(
      candidates, [wantedRole](const auto* candidate) { return candidate->role == wantedRole; });
  if (exactRole != candidates.end()) {
    return *exactRole;
  }
  return candidates.front();
}

} // namespace

TrackSceneBuildResult buildTrackScene(gbx::Challenge challenge, const pak::VirtualFileSystem& vfs,
                                      const LooseFileReader& looseFileReader) {
  if (challenge.mapInfo.collection.empty()) {
    return failed("challenge collection is unavailable");
  }

  TrackSceneBuildResult result;
  result.scene.challenge = std::move(challenge);
  std::unordered_map<std::string, BlockAssetGraph> graphCache;
  std::unordered_map<std::string, std::size_t> modelCache;

  for (std::size_t blockIndex = 0; blockIndex < result.scene.challenge.blocks.size();
       ++blockIndex) {
    const gbx::ChallengeBlock& block = result.scene.challenge.blocks[blockIndex];
    auto graph = graphCache.find(block.name);
    if (graph == graphCache.end()) {
      BlockAssetResolveResult resolved =
          resolveBlockAssets(result.scene.challenge.mapInfo.collection, block.name, vfs);
      if (!resolved) {
        return failed("unable to resolve " + block.name + ": " + resolved.error);
      }
      if (!resolved.graph.unresolvedReferences.empty()) {
        return failed("asset graph for " + block.name + " has unresolved references");
      }
      if (expectedWaypointMetadata(block.name) && !resolved.graph.metadata.has_value()) {
        return failed("waypoint metadata is unavailable for " + block.name);
      }
      if (resolved.graph.metadata.has_value() &&
          resolved.graph.metadata->waypointType != gbx::BlockWaypointType::Start &&
          resolved.graph.triggerSolids.empty()) {
        return failed("waypoint trigger is unavailable for " + block.name);
      }
      if (resolved.graph.metadata.has_value()) {
        result.scene.blockDefinitions.push_back({block.name, std::move(*resolved.graph.metadata)});
      }
      graph = graphCache.emplace(block.name, std::move(resolved.graph)).first;
    }

    if (!graph->second.triggerSolids.empty()) {
      const BlockSolidAsset* const selectedTrigger =
          selectSolid(graph->second.triggerSolids, graph->second.blockName, block);
      if (selectedTrigger == nullptr) {
        return failed("no matching trigger solid is available for " + block.name);
      }
      pak::VfsOpenResult opened = vfs.openFile(selectedTrigger->logicalPath);
      if (!opened) {
        return failed("unable to open " + selectedTrigger->logicalPath + ": " + opened.error);
      }
      gbx::SolidParseResult parsed = gbx::parseSolid(*opened.input);
      if (!parsed) {
        return failed("unable to parse " + selectedTrigger->logicalPath + ": " + parsed.error);
      }
      result.scene.triggers.push_back(
          {blockIndex, selectedTrigger->logicalPath, std::move(parsed.solid)});
    }
    if (graph->second.solids.empty()) {
      ++result.scene.invisibleBlockCount;
      continue;
    }

    const bool ground = (block.flags & kChallengeBlockGroundFlag) != 0U;
    const BlockSolidAsset* const selected =
        selectSolid(graph->second.solids, graph->second.blockName, block);
    if (selected == nullptr) {
      return failed("no " + std::string(ground ? "ground" : "air") + " solid is available for " +
                    block.name);
    }
    const std::string modelKey = pak::normalizeArchivePath(selected->logicalPath);
    auto model = modelCache.find(modelKey);
    if (model == modelCache.end()) {
      pak::VfsOpenResult opened = vfs.openFile(selected->logicalPath);
      if (!opened) {
        return failed("unable to open " + selected->logicalPath + ": " + opened.error);
      }
      gbx::SolidParseResult parsed = gbx::parseSolid(*opened.input);
      if (!parsed) {
        return failed("unable to parse " + selected->logicalPath + ": " + parsed.error);
      }
      const std::size_t modelIndex = result.scene.models.size();
      TrackModel trackModel;
      trackModel.logicalPath = selected->logicalPath;
      trackModel.solid = std::move(parsed.solid);
      result.scene.models.push_back(std::move(trackModel));
      model = modelCache.emplace(modelKey, modelIndex).first;
    }
    result.scene.placements.push_back({blockIndex, model->second});
  }

  std::vector<const gbx::SolidModel*> solids;
  solids.reserve(result.scene.models.size());
  for (const TrackModel& model : result.scene.models) {
    solids.push_back(&model.solid);
  }
  SolidMaterialBatchResolveResult materials = resolveSolidMaterialBatch(
      std::span<const gbx::SolidModel* const>(solids), vfs, looseFileReader);
  if (!materials) {
    return failed("unable to resolve track materials: " + materials.error);
  }
  result.scene.textures = std::move(materials.materials.textures);
  for (std::size_t index = 0; index < result.scene.models.size(); ++index) {
    result.scene.models[index].visualTextureIndices =
        std::move(materials.materials.visualTextureIndices[index]);
  }
  return result;
}

} // namespace tmnf::content
