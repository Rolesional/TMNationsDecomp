#include "tmnf/content/track_collision.hpp"
#include "tmnf/content/track_scene.hpp"
#include "tmnf/gameplay/vehicle.hpp"
#include "tmnf/gbx/challenge.hpp"
#include "tmnf/pak/vfs.hpp"

#include <array>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
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

std::filesystem::path nativePath(const std::string_view virtualPath) {
  std::filesystem::path result;
  std::size_t begin = 0;
  while (begin < virtualPath.size()) {
    const std::size_t separator = virtualPath.find_first_of("\\/", begin);
    const std::size_t end = separator == std::string_view::npos ? virtualPath.size() : separator;
    if (end != begin) {
      result /= std::string(virtualPath.substr(begin, end - begin));
    }
    if (separator == std::string_view::npos) {
      break;
    }
    begin = separator + 1U;
  }
  return result;
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

} // namespace

int main(const int argc, const char* const* argv) {
  if (argc != 5) {
    std::cerr << "usage: archive_track_scene <Stadium.pak> <packlist.dat> <Challenge.Gbx> "
                 "<GameData-root>\n";
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
  tmnf::gbx::ChallengeParseResult parsed = tmnf::gbx::parseChallenge(challengeBytes);
  if (!parsed) {
    std::cerr << parsed.error << '\n';
    return 1;
  }
  const std::filesystem::path gameDataRoot(argv[4]);
  const tmnf::content::LooseFileReader looseReader =
      [&gameDataRoot](const std::string_view path) -> std::optional<std::vector<std::byte>> {
    std::vector<std::byte> bytes;
    if (!readFile(gameDataRoot / nativePath(path), bytes)) {
      return std::nullopt;
    }
    return bytes;
  };
  tmnf::content::TrackSceneBuildResult built =
      tmnf::content::buildTrackScene(std::move(parsed.challenge), vfs, looseReader);
  if (!built) {
    std::cerr << built.error << '\n';
    return 1;
  }
  const tmnf::content::TrackScene& scene = built.scene;
  const tmnf::content::TrackCollisionBuildResult collisionBuilt =
      tmnf::content::buildTrackCollision(scene);
  if (!collisionBuilt) {
    std::cerr << collisionBuilt.error << '\n';
    return 1;
  }
  const tmnf::content::TrackCollision& collision = collisionBuilt.collision;
  tmnf::gameplay::VehicleSimulation vehicle;
  const tmnf::gameplay::VehicleInitializeResult vehicleInitialized =
      vehicle.initialize(scene, collision);
  if (!vehicleInitialized) {
    std::cerr << vehicleInitialized.error << '\n';
    return 1;
  }
  const tmnf::gameplay::VehicleState& vehicleSpawn = vehicle.state();
  std::size_t occupiedCollisionCells = 0;
  for (const auto& cell : collision.cellTriangles) {
    occupiedCollisionCells += cell.empty() ? 0U : 1U;
  }
  const tmnf::gbx::ChallengeBlock* startBlock = nullptr;
  for (const tmnf::gbx::ChallengeBlock& block : scene.challenge.blocks) {
    if (block.name == "StadiumRoadMainStartLine") {
      startBlock = &block;
      break;
    }
  }
  std::optional<tmnf::content::TrackGroundHit> startGround;
  tmnf::gbx::Float3 startCenter;
  std::array<std::size_t, 4> startGroundSamples{};
  if (startBlock != nullptr) {
    startCenter = vehicleSpawn.position;
    startGround = tmnf::content::sampleTrackGround(collision, startCenter.x, startCenter.z,
                                                   startCenter.y - 16.0F, startCenter.y + 32.0F);
    constexpr std::array<tmnf::gbx::Float3, 4> directions{
        tmnf::gbx::Float3{1.0F, 0.0F, 0.0F}, tmnf::gbx::Float3{-1.0F, 0.0F, 0.0F},
        tmnf::gbx::Float3{0.0F, 0.0F, 1.0F}, tmnf::gbx::Float3{0.0F, 0.0F, -1.0F}};
    for (std::size_t direction = 0; direction < directions.size(); ++direction) {
      for (float distance = 4.0F; distance <= 64.0F; distance += 4.0F) {
        const auto hit = tmnf::content::sampleTrackGround(
            collision, startCenter.x + directions[direction].x * distance,
            startCenter.z + directions[direction].z * distance, startCenter.y - 4.0F,
            startCenter.y + 4.0F);
        startGroundSamples[direction] += hit.has_value() ? 1U : 0U;
      }
    }
  }
  std::vector<std::size_t> modelPlacementCounts(scene.models.size());
  for (const tmnf::content::TrackPlacement& placement : scene.placements) {
    ++modelPlacementCounts[placement.modelIndex];
  }
  std::size_t uniqueVisuals = 0;
  std::size_t uniqueVertices = 0;
  std::size_t uniqueIndices = 0;
  std::size_t instancedVertices = 0;
  std::size_t instancedIndices = 0;
  std::size_t triggerVertices = 0;
  std::size_t triggerIndices = 0;
  for (const tmnf::content::TrackTriggerMesh& trigger : collision.triggers) {
    triggerVertices += trigger.positions.size();
    triggerIndices += trigger.indices.size();
  }
  for (std::size_t modelIndex = 0; modelIndex < scene.models.size(); ++modelIndex) {
    const tmnf::content::TrackModel& model = scene.models[modelIndex];
    for (const tmnf::gbx::SolidVisual& visual : model.solid.visuals) {
      if (!visual.renderEnabled) {
        continue;
      }
      ++uniqueVisuals;
      uniqueVertices += visual.positions.size();
      uniqueIndices += visual.indices.size();
      instancedVertices += visual.positions.size() * modelPlacementCounts[modelIndex];
      instancedIndices += visual.indices.size() * modelPlacementCounts[modelIndex];
    }
  }

  std::cout << "{\n"
            << "  \"mapName\": \"" << jsonEscape(scene.challenge.mapName) << "\",\n"
            << "  \"challengeBlockCount\": " << scene.challenge.blocks.size() << ",\n"
            << "  \"placementCount\": " << scene.placements.size() << ",\n"
            << "  \"invisibleBlockCount\": " << scene.invisibleBlockCount << ",\n"
            << "  \"modelCount\": " << scene.models.size() << ",\n"
            << "  \"textureCount\": " << scene.textures.size() << ",\n"
            << "  \"uniqueEnabledVisualCount\": " << uniqueVisuals << ",\n"
            << "  \"uniqueVertexCount\": " << uniqueVertices << ",\n"
            << "  \"uniqueIndexCount\": " << uniqueIndices << ",\n"
            << "  \"instancedVertexCount\": " << instancedVertices << ",\n"
            << "  \"instancedIndexCount\": " << instancedIndices << ",\n"
            << "  \"collisionVertexCount\": " << collision.positions.size() << ",\n"
            << "  \"collisionIndexCount\": " << collision.indices.size() << ",\n"
            << "  \"triggerCount\": " << collision.triggers.size() << ",\n"
            << "  \"triggerVertexCount\": " << triggerVertices << ",\n"
            << "  \"triggerIndexCount\": " << triggerIndices << ",\n"
            << "  \"orderedCheckpointCount\": " << scene.challenge.checkpoints.size() << ",\n"
            << "  \"vehicleCheckpointCount\": " << vehicle.checkpointTotal() << ",\n"
            << "  \"occupiedCollisionCellCount\": " << occupiedCollisionCells << ",\n"
            << "  \"vehicleSpawn\": {\"x\": " << vehicleSpawn.position.x
            << ", \"y\": " << vehicleSpawn.position.y << ", \"z\": " << vehicleSpawn.position.z
            << ", \"heading\": " << vehicleSpawn.heading << "},\n"
            << "  \"startGround\": {\"x\": " << startCenter.x << ", \"z\": " << startCenter.z
            << ", \"height\": " << (startGround.has_value() ? startGround->height : -1.0F)
            << ", \"surfaceId\": " << (startGround.has_value() ? startGround->surfaceId : -1)
            << "},\n"
            << "  \"startGroundSamples\": {\"positiveX\": " << startGroundSamples[0]
            << ", \"negativeX\": " << startGroundSamples[1]
            << ", \"positiveZ\": " << startGroundSamples[2]
            << ", \"negativeZ\": " << startGroundSamples[3] << "},\n"
            << "  \"models\": [\n";
  for (std::size_t modelIndex = 0; modelIndex < scene.models.size(); ++modelIndex) {
    const tmnf::content::TrackModel& model = scene.models[modelIndex];
    std::size_t enabled = 0;
    for (const tmnf::gbx::SolidVisual& visual : model.solid.visuals) {
      enabled += visual.renderEnabled ? 1U : 0U;
    }
    std::cout << "    {\"path\": \"" << jsonEscape(model.logicalPath)
              << "\", \"placements\": " << modelPlacementCounts[modelIndex]
              << ", \"visuals\": " << model.solid.visuals.size()
              << ", \"enabledVisuals\": " << enabled << "}"
              << (modelIndex + 1U == scene.models.size() ? "" : ",") << '\n';
  }
  std::cout << "  ],\n  \"textures\": [\n";
  for (std::size_t index = 0; index < scene.textures.size(); ++index) {
    const tmnf::content::ResolvedMaterialTexture& texture = scene.textures[index];
    std::cout << "    {\"image\": \"" << jsonEscape(texture.imagePath)
              << "\", \"width\": " << texture.image.width
              << ", \"height\": " << texture.image.height << "}"
              << (index + 1U == scene.textures.size() ? "" : ",") << '\n';
  }
  std::cout << "  ]\n}\n";
  return 0;
}
