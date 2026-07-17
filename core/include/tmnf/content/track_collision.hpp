#pragma once

#include "tmnf/content/track_scene.hpp"
#include "tmnf/gbx/solid.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace tmnf::content {

inline constexpr float kTrackHorizontalCellSize = 32.0F;
inline constexpr float kTrackVerticalCellSize = 8.0F;

struct TrackTriggerMesh final {
  std::size_t blockIndex = 0;
  std::vector<gbx::Float3> positions;
  std::vector<std::uint32_t> indices;
};

struct TrackCollision final {
  std::vector<gbx::Float3> positions;
  std::vector<std::uint32_t> indices;
  std::vector<std::int16_t> triangleSurfaceIds;
  std::array<float, 6> bounds{};
  std::int32_t gridWidth = 0;
  std::int32_t gridDepth = 0;
  std::vector<std::vector<std::uint32_t>> cellTriangles;
  std::vector<TrackTriggerMesh> triggers;
};

struct TrackCollisionBuildResult final {
  TrackCollision collision;
  std::string error;

  [[nodiscard]] explicit operator bool() const noexcept { return error.empty(); }
};

struct TrackGroundHit final {
  float height = 0.0F;
  gbx::Float3 normal{0.0F, 1.0F, 0.0F};
  std::int16_t surfaceId = -1;
  std::uint32_t triangleIndex = 0;
};

struct TrackWallContact final {
  gbx::Float3 normal;
  float penetration = 0.0F;
  std::int16_t surfaceId = -1;
  std::uint32_t triangleIndex = 0;
};

[[nodiscard]] gbx::Float3 transformIso4Point(const std::array<float, 12>& transform,
                                             gbx::Float3 point) noexcept;
[[nodiscard]] gbx::Float3 rotateTrackVector(gbx::Float3 value, std::uint8_t direction) noexcept;
[[nodiscard]] gbx::Float3 placeTrackPoint(gbx::Float3 value,
                                          const gbx::ChallengeBlock& block) noexcept;

// Instantiates every selected CPlugSurfaceGeom mesh in challenge space and
// builds an X/Z cell index for deterministic vehicle-to-track queries.
[[nodiscard]] TrackCollisionBuildResult buildTrackCollision(const TrackScene& scene);

// Returns the highest non-vertical triangle under X/Z within the inclusive Y range.
[[nodiscard]] std::optional<TrackGroundHit> sampleTrackGround(const TrackCollision& collision,
                                                              float x, float z, float minimumY,
                                                              float maximumY) noexcept;

// Returns the deepest overlap between a sphere and a non-ground collision triangle.
// The returned normal is horizontal and points away from the contacted surface.
[[nodiscard]] std::optional<TrackWallContact>
sampleTrackWallContact(const TrackCollision& collision, gbx::Float3 center, float radius) noexcept;

// Sweeps a point segment against a trigger mesh. Triangle winding is ignored.
[[nodiscard]] bool segmentIntersectsTrackTrigger(const TrackTriggerMesh& trigger, gbx::Float3 from,
                                                 gbx::Float3 to) noexcept;

} // namespace tmnf::content
