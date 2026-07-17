#pragma once

#include "tmnf/gbx/input.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <string>

namespace tmnf::gbx {

enum class BlockWaypointType : std::int32_t {
  Start = 0,
  Finish = 1,
  Checkpoint = 2,
  None = 3,
  StartFinish = 4,
  Dispenser = 5,
};

struct BlockInfoMetadata final {
  std::optional<std::array<float, 12>> spawnGround;
  std::optional<std::array<float, 12>> spawnAir;
  BlockWaypointType waypointType = BlockWaypointType::None;
  bool noRespawn = false;
};

struct BlockInfoParseResult final {
  BlockInfoMetadata blockInfo;
  std::string error;

  [[nodiscard]] explicit operator bool() const noexcept { return error.empty(); }
};

// Reads the TMF CGameCtnBlockInfo metadata needed by gameplay. Inline
// CGameCtnBlockUnitInfo nodes are consumed with the same parser-timed dummy writes
// used by NadeoPak, including waypoint helper mobils, through NoRespawn.
[[nodiscard]] BlockInfoParseResult parseBlockInfo(Input& input);

} // namespace tmnf::gbx
