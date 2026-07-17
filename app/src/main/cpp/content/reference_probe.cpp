#include "content/reference_probe.hpp"

#include "tmnf/content/track_scene.hpp"
#include "tmnf/gbx/campaign.hpp"
#include "tmnf/gbx/challenge.hpp"
#include "tmnf/image/dds.hpp"
#include "tmnf/pak/vfs.hpp"

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string_view>
#include <system_error>
#include <unordered_map>
#include <utility>
#include <vector>

namespace tmnf::content {
namespace {

constexpr std::string_view kTexturePath = "Media\\Texture\\Image\\MeshDefault.dds";
constexpr std::string_view kCampaignPath =
    "Tracks\\Campaigns\\Nations\\Nations.ConstructionCampaign.Gbx";
constexpr std::string_view kStaleCampaignUid = "Nk8jftxeWaIICIOEGV8M_cV3gba";
constexpr std::string_view kVehicleSolidPath = "Vehicles\\Media\\Solid\\StadiumCar.Solid.Gbx";
constexpr std::string_view kCheckpointSoundPath =
    "Interface\\Media\\Audio\\Sound\\Wav\\RaceCheckPoint.wav";
constexpr std::string_view kFinishSoundPath = "Interface\\Media\\Audio\\Sound\\Wav\\RaceFinish.wav";
constexpr std::string_view kHudFontPath = "Interface\\Media\\Font\\Led_Textures\\Led_00.dds";

ReferenceProbeResult probeFail(std::string message) {
  ReferenceProbeResult result;
  result.error = std::move(message);
  return result;
}

ReferenceCampaignResult campaignFail(std::string message) {
  ReferenceCampaignResult result;
  result.error = std::move(message);
  return result;
}

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

bool endsWithChallengeExtension(const std::string_view name) noexcept {
  constexpr std::string_view suffix = ".Challenge.Gbx";
  if (name.size() < suffix.size()) {
    return false;
  }
  return std::equal(
      suffix.rbegin(), suffix.rend(), name.rbegin(), [](const char left, const char right) {
        const auto lower = [](const char value) {
          return value >= 'A' && value <= 'Z' ? static_cast<char>(value + ('a' - 'A')) : value;
        };
        return lower(left) == lower(right);
      });
}

} // namespace

ReferenceCampaignResult loadReferenceCampaign(const char* const externalDataPath) {
  if (externalDataPath == nullptr || *externalDataPath == '\0') {
    return campaignFail("Android external data path is unavailable");
  }
  const std::filesystem::path gameDataRoot = std::filesystem::path(externalDataPath) / "GameData";
  std::vector<std::byte> campaignRaw;
  if (!readFile(gameDataRoot / nativePath(kCampaignPath), campaignRaw)) {
    return campaignFail("Nations campaign container is not installed");
  }
  gbx::CampaignParseResult parsedCampaign = gbx::parseCampaign(campaignRaw);
  if (!parsedCampaign) {
    return campaignFail("Nations campaign parse failed: " + parsedCampaign.error);
  }

  std::unordered_map<std::string, ReferenceCampaignMap> looseMaps;
  const std::filesystem::path nationsRoot = gameDataRoot / nativePath("Tracks\\Campaigns\\Nations");
  std::error_code error;
  std::filesystem::recursive_directory_iterator iterator(
      nationsRoot, std::filesystem::directory_options::skip_permission_denied, error);
  const std::filesystem::recursive_directory_iterator end;
  while (!error && iterator != end) {
    if (iterator->is_regular_file(error) &&
        endsWithChallengeExtension(iterator->path().filename().string())) {
      std::vector<std::byte> raw;
      if (!readFile(iterator->path(), raw)) {
        return campaignFail("unable to read a Nations challenge header");
      }
      gbx::ChallengeHeaderParseResult header = gbx::parseChallengeHeaderMetadata(raw);
      if (!header) {
        return campaignFail("Nations challenge header parse failed: " + header.error);
      }
      std::error_code relativeError;
      std::string relativePath =
          std::filesystem::relative(iterator->path(), gameDataRoot, relativeError).generic_string();
      if (relativeError) {
        return campaignFail("unable to form a Nations challenge path");
      }
      ReferenceCampaignMap map;
      map.relativePath = std::move(relativePath);
      map.metadata = std::move(header.metadata);
      const std::string uid = map.metadata.mapInfo.id;
      if (!looseMaps.emplace(uid, std::move(map)).second) {
        return campaignFail("duplicate Nations challenge UID: " + uid);
      }
    }
    iterator.increment(error);
  }
  if (error) {
    return campaignFail("unable to enumerate Nations challenges: " + error.message());
  }

  ReferenceCampaignResult result;
  std::size_t missingCount = 0;
  std::string missingUid;
  for (const gbx::CampaignGroup& group : parsedCampaign.campaign.groups) {
    for (const gbx::ChallengeIdent& identity : group.maps) {
      const auto found = looseMaps.find(identity.id);
      if (found == looseMaps.end()) {
        ++missingCount;
        missingUid = identity.id;
        continue;
      }
      // The official container preserves creator names that intentionally differ from some
      // normalized loose challenge headers; the stable MapInfo UID is the source join key.
      found->second.groupName = group.name;
      result.maps.push_back(std::move(found->second));
      looseMaps.erase(found);
    }
  }
  if (result.maps.size() != 65U || !looseMaps.empty() || missingCount != 1U ||
      missingUid != kStaleCampaignUid) {
    return campaignFail("Nations campaign did not resolve to its 65 official loose challenges");
  }
  return result;
}

ReferenceProbeResult loadReferenceContent(const char* const externalDataPath,
                                          const std::string_view challengePath) {
  if (externalDataPath == nullptr || *externalDataPath == '\0') {
    return probeFail("Android external data path is unavailable");
  }
  const std::filesystem::path packRoot = std::filesystem::path(externalDataPath) / "Packs";
  std::vector<std::byte> packListRaw;
  std::vector<std::byte> resourceRaw;
  std::vector<std::byte> stadiumRaw;
  if (!readFile(packRoot / "packlist.dat", packListRaw)) {
    return probeFail("Packs/packlist.dat is not installed");
  }
  if (!readFile(packRoot / "Resource.pak", resourceRaw)) {
    return probeFail("Packs/Resource.pak is not installed");
  }
  if (!readFile(packRoot / "Stadium.pak", stadiumRaw)) {
    return probeFail("Packs/Stadium.pak is not installed");
  }

  pak::VirtualFileSystem vfs;
  const pak::VfsOperationResult initialized = vfs.initialize(packListRaw);
  if (!initialized) {
    return probeFail("VFS initialization failed: " + initialized.error);
  }
  const pak::VfsOperationResult resourceMounted = vfs.mount("resource", std::move(resourceRaw));
  if (!resourceMounted) {
    return probeFail("Resource.pak mount failed: " + resourceMounted.error);
  }
  const pak::VfsOperationResult stadiumMounted = vfs.mount("stadium", std::move(stadiumRaw));
  if (!stadiumMounted) {
    return probeFail("Stadium.pak mount failed: " + stadiumMounted.error);
  }

  pak::VfsReadResult fileData = vfs.readFile(kTexturePath);
  if (!fileData) {
    return probeFail("reference texture extraction failed: " + fileData.error);
  }
  image::DecodeResult decoded = image::decodeDds(fileData.data);
  if (!decoded) {
    return probeFail("reference DDS decode failed: " + decoded.error);
  }

  const std::filesystem::path gameDataRoot = std::filesystem::path(externalDataPath) / "GameData";
  std::vector<std::byte> checkpointSound;
  std::vector<std::byte> finishSound;
  image::RgbaImage hudFont;
  (void)readFile(gameDataRoot / nativePath(kCheckpointSoundPath), checkpointSound);
  (void)readFile(gameDataRoot / nativePath(kFinishSoundPath), finishSound);
  std::vector<std::byte> hudFontRaw;
  if (readFile(gameDataRoot / nativePath(kHudFontPath), hudFontRaw)) {
    image::DecodeResult decodedHudFont = image::decodeDds(hudFontRaw);
    if (!decodedHudFont) {
      return probeFail("LED HUD font decode failed: " + decodedHudFont.error);
    }
    hudFont = std::move(decodedHudFont.image);
  }
  std::vector<std::byte> challengeRaw;
  if (!readFile(gameDataRoot / nativePath(challengePath), challengeRaw)) {
    return probeFail("campaign challenge is not installed: " + std::string(challengePath));
  }
  gbx::ChallengeParseResult parsedChallenge = gbx::parseChallenge(challengeRaw);
  if (!parsedChallenge) {
    return probeFail("campaign challenge parse failed: " + parsedChallenge.error);
  }
  const LooseFileReader looseReader =
      [&gameDataRoot](const std::string_view path) -> std::optional<std::vector<std::byte>> {
    std::vector<std::byte> bytes;
    if (!readFile(gameDataRoot / nativePath(path), bytes)) {
      return std::nullopt;
    }
    return bytes;
  };
  TrackSceneBuildResult builtScene =
      buildTrackScene(std::move(parsedChallenge.challenge), vfs, looseReader);
  if (!builtScene) {
    return probeFail("campaign scene construction failed: " + builtScene.error);
  }

  pak::VfsOpenResult vehicleInput = vfs.openFile(kVehicleSolidPath);
  if (!vehicleInput) {
    return probeFail("StadiumCar solid is unavailable: " + vehicleInput.error);
  }
  gbx::SolidParseResult vehicle = gbx::parseSolid(*vehicleInput.input);
  if (!vehicle) {
    return probeFail("StadiumCar solid parse failed: " + vehicle.error);
  }
  std::erase_if(vehicle.solid.visuals, [](const gbx::SolidVisual& visual) {
    return visual.treeName.empty() || visual.treeName.front() != '3';
  });
  if (vehicle.solid.visuals.size() != 7U) {
    return probeFail("StadiumCar low LOD does not contain seven visuals");
  }
  for (gbx::SolidVisual& visual : vehicle.solid.visuals) {
    if (visual.materialReference.starts_with("Material\\")) {
      visual.materialReference = "Vehicles\\Media\\" + visual.materialReference;
    }
  }
  SolidMaterialResolveResult vehicleMaterials =
      resolveSolidMaterials(vehicle.solid, vfs, looseReader);
  if (!vehicleMaterials) {
    return probeFail("StadiumCar materials failed: " + vehicleMaterials.error);
  }
  ReferenceProbeResult result;
  result.image = std::move(decoded.image);
  result.hudFont = std::move(hudFont);
  result.scene = std::move(builtScene.scene);
  result.vehicleSolid = std::move(vehicle.solid);
  result.vehicleMaterials = std::move(vehicleMaterials.materials);
  result.checkpointSound = std::move(checkpointSound);
  result.finishSound = std::move(finishSound);
  return result;
}

} // namespace tmnf::content
