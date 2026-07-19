#pragma once

#include "tmnf/content/materials.hpp"
#include "tmnf/gbx/block_info.hpp"
#include "tmnf/gbx/challenge.hpp"
#include "tmnf/gbx/solid.hpp"
#include "tmnf/pak/vfs.hpp"

#include <array>
#include <cstddef>
#include <string>
#include <vector>

namespace tmnf::content {

struct TrackModel final {
  std::string logicalPath;
  gbx::SolidModel solid;
  // One global TrackScene::textures index per SolidModel::visuals element.
  std::vector<std::size_t> visualTextureIndices;
};

struct TrackPlacement final {
  std::size_t blockIndex = 0;
  std::size_t modelIndex = 0;
  std::array<std::int32_t, 2> footprint{1, 1};
};

struct TrackTrigger final {
  std::size_t blockIndex = 0;
  std::string logicalPath;
  gbx::SolidModel solid;
  std::array<std::int32_t, 2> footprint{1, 1};
};

struct TrackBlockDefinition final {
  std::string name;
  gbx::BlockInfoMetadata metadata;
};

struct TrackScene final {
  gbx::Challenge challenge;
  std::vector<TrackBlockDefinition> blockDefinitions;
  std::vector<TrackModel> models;
  std::vector<ResolvedMaterialTexture> textures;
  std::vector<TrackPlacement> placements;
  std::vector<TrackTrigger> triggers;
  std::size_t invisibleBlockCount = 0;
};

struct TrackSceneBuildResult final {
  TrackScene scene;
  std::string error;

  [[nodiscard]] explicit operator bool() const noexcept { return error.empty(); }
};

// Resolves construction assets for each challenge block, selects the placement's
// ground/air variant, parses each unique solid once, and shares decoded textures
// across the complete scene. Empty BlockInfo graphs represent intentionally
// invisible editor/connectivity blocks and are retained only in the challenge.
[[nodiscard]] TrackSceneBuildResult buildTrackScene(gbx::Challenge challenge,
                                                    const pak::VirtualFileSystem& vfs,
                                                    const LooseFileReader& looseFileReader);

} // namespace tmnf::content
