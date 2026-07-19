#pragma once

#include "tmnf/content/track_scene.hpp"
#include "tmnf/gbx/challenge.hpp"
#include "tmnf/gbx/sound_engine.hpp"
#include "tmnf/gbx/vehicle_tunings.hpp"
#include "tmnf/image/image.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace tmnf::content {

struct ReferenceProbeResult final {
  image::RgbaImage image;
  image::RgbaImage hudFont;
  TrackScene scene;
  gbx::SolidModel vehicleSolid;
  SolidMaterialSet vehicleMaterials;
  gbx::VehicleTunings vehicleTunings;
  gbx::SoundEngine soundEngine;
  std::vector<std::vector<std::byte>> engineSounds;
  std::vector<std::byte> checkpointSound;
  std::vector<std::byte> finishSound;
  std::vector<std::byte> countdown1Sound;
  std::vector<std::byte> countdown2Sound;
  std::vector<std::byte> countdown3Sound;
  std::vector<std::byte> goSound;
  std::string error;

  [[nodiscard]] explicit operator bool() const noexcept { return error.empty(); }
};

struct ReferenceCampaignMap final {
  std::string groupName;
  std::string relativePath;
  gbx::ChallengeHeaderMetadata metadata;
};

struct ReferenceCampaignResult final {
  std::vector<ReferenceCampaignMap> maps;
  std::string error;

  [[nodiscard]] explicit operator bool() const noexcept { return error.empty(); }
};

// Joins the official campaign container to loose challenges by source UID,
// preserving group/map order and omitting the container's one stale Red UID.
[[nodiscard]] ReferenceCampaignResult loadReferenceCampaign(const char* dataRootPath);

// Loads a loose campaign challenge plus packed Stadium content through the same
// portable parsers/VFS path used by the port. No original asset is built into the APK.
[[nodiscard]] ReferenceProbeResult loadReferenceContent(const char* dataRootPath,
                                                        std::string_view challengePath);

} // namespace tmnf::content
