#include "tmnf/content/track_collision.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace tmnf::content {
namespace {

TrackCollisionBuildResult failed(std::string message) {
  TrackCollisionBuildResult result;
  result.error = std::move(message);
  return result;
}

gbx::Float3 subtract(const gbx::Float3 left, const gbx::Float3 right) noexcept {
  return {left.x - right.x, left.y - right.y, left.z - right.z};
}

gbx::Float3 cross(const gbx::Float3 left, const gbx::Float3 right) noexcept {
  return {left.y * right.z - left.z * right.y, left.z * right.x - left.x * right.z,
          left.x * right.y - left.y * right.x};
}

float lengthSquared(const gbx::Float3 value) noexcept {
  return value.x * value.x + value.y * value.y + value.z * value.z;
}

float dot(const gbx::Float3 left, const gbx::Float3 right) noexcept {
  return left.x * right.x + left.y * right.y + left.z * right.z;
}

gbx::Float3 add(const gbx::Float3 left, const gbx::Float3 right) noexcept {
  return {left.x + right.x, left.y + right.y, left.z + right.z};
}

gbx::Float3 multiply(const gbx::Float3 value, const float scalar) noexcept {
  return {value.x * scalar, value.y * scalar, value.z * scalar};
}

gbx::Float3 closestPointOnTriangle(const gbx::Float3 point, const gbx::Float3 a,
                                   const gbx::Float3 b, const gbx::Float3 c) noexcept {
  const gbx::Float3 ab = subtract(b, a);
  const gbx::Float3 ac = subtract(c, a);
  const gbx::Float3 ap = subtract(point, a);
  const float d1 = dot(ab, ap);
  const float d2 = dot(ac, ap);
  if (d1 <= 0.0F && d2 <= 0.0F) {
    return a;
  }

  const gbx::Float3 bp = subtract(point, b);
  const float d3 = dot(ab, bp);
  const float d4 = dot(ac, bp);
  if (d3 >= 0.0F && d4 <= d3) {
    return b;
  }

  const float vc = d1 * d4 - d3 * d2;
  if (vc <= 0.0F && d1 >= 0.0F && d3 <= 0.0F) {
    return add(a, multiply(ab, d1 / (d1 - d3)));
  }

  const gbx::Float3 cp = subtract(point, c);
  const float d5 = dot(ab, cp);
  const float d6 = dot(ac, cp);
  if (d6 >= 0.0F && d5 <= d6) {
    return c;
  }

  const float vb = d5 * d2 - d1 * d6;
  if (vb <= 0.0F && d2 >= 0.0F && d6 <= 0.0F) {
    return add(a, multiply(ac, d2 / (d2 - d6)));
  }

  const float va = d3 * d6 - d5 * d4;
  if (va <= 0.0F && d4 - d3 >= 0.0F && d5 - d6 >= 0.0F) {
    const gbx::Float3 edge = subtract(c, b);
    return add(b, multiply(edge, (d4 - d3) / ((d4 - d3) + (d5 - d6))));
  }

  const float denominator = 1.0F / (va + vb + vc);
  return add(a, add(multiply(ab, vb * denominator), multiply(ac, vc * denominator)));
}

gbx::Float3 normalize(gbx::Float3 value) noexcept {
  const float squared = lengthSquared(value);
  if (squared <= 1.0e-12F) {
    return {0.0F, 1.0F, 0.0F};
  }
  const float inverse = 1.0F / std::sqrt(squared);
  value.x *= inverse;
  value.y *= inverse;
  value.z *= inverse;
  return value;
}

std::int16_t triangleSurfaceId(const gbx::SolidModel& solid, const gbx::SolidCollisionShape& shape,
                               const std::size_t triangleIndex) {
  if (triangleIndex >= shape.triangleSurfaceIndices.size()) {
    return shape.surfaceId;
  }
  const std::int16_t materialIndex = shape.triangleSurfaceIndices[triangleIndex];
  if (materialIndex >= 0 &&
      static_cast<std::size_t>(materialIndex) < solid.surfaceMaterials.size() &&
      solid.surfaceMaterials[static_cast<std::size_t>(materialIndex)] >= 0) {
    return solid.surfaceMaterials[static_cast<std::size_t>(materialIndex)];
  }
  return shape.surfaceId >= 0 ? shape.surfaceId : materialIndex;
}

} // namespace

gbx::Float3 transformIso4Point(const std::array<float, 12>& transform,
                               const gbx::Float3 point) noexcept {
  return {transform[0] * point.x + transform[1] * point.y + transform[2] * point.z + transform[9],
          transform[3] * point.x + transform[4] * point.y + transform[5] * point.z + transform[10],
          transform[6] * point.x + transform[7] * point.y + transform[8] * point.z + transform[11]};
}

gbx::Float3 rotateTrackVector(const gbx::Float3 value, const std::uint8_t direction) noexcept {
  switch (direction & 3U) {
  case 1:
    return {-value.z, value.y, value.x};
  case 2:
    return {-value.x, value.y, -value.z};
  case 3:
    return {value.z, value.y, -value.x};
  default:
    return value;
  }
}

gbx::Float3 placeTrackPoint(const gbx::Float3 value, const gbx::ChallengeBlock& block) noexcept {
  constexpr gbx::Float3 center{kTrackHorizontalCellSize * 0.5F, 0.0F,
                               kTrackHorizontalCellSize * 0.5F};
  gbx::Float3 centered{value.x - center.x, value.y, value.z - center.z};
  const gbx::Float3 rotated = rotateTrackVector(centered, block.direction);
  return {rotated.x + center.x + static_cast<float>(block.coordinate[0]) * kTrackHorizontalCellSize,
          rotated.y + static_cast<float>(block.coordinate[1]) * kTrackVerticalCellSize,
          rotated.z + center.z +
              static_cast<float>(block.coordinate[2]) * kTrackHorizontalCellSize};
}

TrackCollisionBuildResult buildTrackCollision(const TrackScene& scene) {
  if (scene.challenge.size[0] <= 0 || scene.challenge.size[2] <= 0 ||
      scene.challenge.size[0] > 1024 || scene.challenge.size[2] > 1024) {
    return failed("challenge collision grid dimensions are invalid");
  }
  TrackCollisionBuildResult result;
  TrackCollision& collision = result.collision;
  collision.gridWidth = scene.challenge.size[0];
  collision.gridDepth = scene.challenge.size[2];
  collision.cellTriangles.resize(static_cast<std::size_t>(collision.gridWidth) *
                                 static_cast<std::size_t>(collision.gridDepth));

  collision.bounds = {std::numeric_limits<float>::max(),    std::numeric_limits<float>::max(),
                      std::numeric_limits<float>::max(),    std::numeric_limits<float>::lowest(),
                      std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()};
  for (const TrackPlacement& placement : scene.placements) {
    if (placement.blockIndex >= scene.challenge.blocks.size() ||
        placement.modelIndex >= scene.models.size()) {
      return failed("track placement references an invalid block or model");
    }
    const gbx::ChallengeBlock& block = scene.challenge.blocks[placement.blockIndex];
    const gbx::SolidModel& solid = scene.models[placement.modelIndex].solid;
    for (const gbx::SolidCollisionShape& shape : solid.collisionShapes) {
      if (shape.type != gbx::SolidCollisionType::Mesh) {
        return failed("track collision contains an unsupported non-mesh surface");
      }
      if (shape.indices.size() % 3U != 0U ||
          shape.triangleSurfaceIndices.size() != shape.indices.size() / 3U) {
        return failed("track collision mesh has inconsistent triangle data");
      }
      if (collision.positions.size() >
          std::numeric_limits<std::uint32_t>::max() - shape.positions.size()) {
        return failed("track collision vertex count exceeds 32-bit indexing");
      }
      const std::uint32_t vertexBase = static_cast<std::uint32_t>(collision.positions.size());
      for (const gbx::Float3 source : shape.positions) {
        const gbx::Float3 position =
            placeTrackPoint(transformIso4Point(shape.transform, source), block);
        collision.positions.push_back(position);
        collision.bounds[0] = std::min(collision.bounds[0], position.x);
        collision.bounds[1] = std::min(collision.bounds[1], position.y);
        collision.bounds[2] = std::min(collision.bounds[2], position.z);
        collision.bounds[3] = std::max(collision.bounds[3], position.x);
        collision.bounds[4] = std::max(collision.bounds[4], position.y);
        collision.bounds[5] = std::max(collision.bounds[5], position.z);
      }
      for (const std::uint32_t index : shape.indices) {
        if (index >= shape.positions.size()) {
          return failed("track collision mesh index is out of bounds");
        }
        collision.indices.push_back(vertexBase + index);
      }
      for (std::size_t triangle = 0; triangle < shape.indices.size() / 3U; ++triangle) {
        collision.triangleSurfaceIds.push_back(triangleSurfaceId(solid, shape, triangle));
      }
    }
  }
  for (const TrackTrigger& sourceTrigger : scene.triggers) {
    if (sourceTrigger.blockIndex >= scene.challenge.blocks.size()) {
      return failed("track trigger references an invalid block");
    }
    const gbx::ChallengeBlock& block = scene.challenge.blocks[sourceTrigger.blockIndex];
    TrackTriggerMesh trigger;
    trigger.blockIndex = sourceTrigger.blockIndex;
    for (const gbx::SolidCollisionShape& shape : sourceTrigger.solid.collisionShapes) {
      if (shape.type != gbx::SolidCollisionType::Mesh || shape.indices.size() % 3U != 0U) {
        return failed("track trigger contains invalid non-triangle geometry");
      }
      if (trigger.positions.size() >
          std::numeric_limits<std::uint32_t>::max() - shape.positions.size()) {
        return failed("track trigger vertex count exceeds 32-bit indexing");
      }
      const std::uint32_t vertexBase = static_cast<std::uint32_t>(trigger.positions.size());
      for (const gbx::Float3 source : shape.positions) {
        trigger.positions.push_back(
            placeTrackPoint(transformIso4Point(shape.transform, source), block));
      }
      for (const std::uint32_t index : shape.indices) {
        if (index >= shape.positions.size()) {
          return failed("track trigger mesh index is out of bounds");
        }
        trigger.indices.push_back(vertexBase + index);
      }
    }
    if (trigger.indices.empty()) {
      return failed("track trigger has no collision triangles");
    }
    collision.triggers.push_back(std::move(trigger));
  }
  if (collision.indices.empty()) {
    return failed("track scene has no collision triangles");
  }

  for (std::uint32_t triangleIndex = 0;
       triangleIndex < static_cast<std::uint32_t>(collision.indices.size() / 3U); ++triangleIndex) {
    const gbx::Float3 a = collision.positions[collision.indices[triangleIndex * 3U]];
    const gbx::Float3 b = collision.positions[collision.indices[triangleIndex * 3U + 1U]];
    const gbx::Float3 c = collision.positions[collision.indices[triangleIndex * 3U + 2U]];
    const float minimumX = std::min({a.x, b.x, c.x});
    const float maximumX = std::max({a.x, b.x, c.x});
    const float minimumZ = std::min({a.z, b.z, c.z});
    const float maximumZ = std::max({a.z, b.z, c.z});
    const int firstX =
        std::max(0, static_cast<int>(std::floor(minimumX / kTrackHorizontalCellSize)));
    const int lastX = std::min(collision.gridWidth - 1,
                               static_cast<int>(std::floor(maximumX / kTrackHorizontalCellSize)));
    const int firstZ =
        std::max(0, static_cast<int>(std::floor(minimumZ / kTrackHorizontalCellSize)));
    const int lastZ = std::min(collision.gridDepth - 1,
                               static_cast<int>(std::floor(maximumZ / kTrackHorizontalCellSize)));
    for (int z = firstZ; z <= lastZ; ++z) {
      for (int x = firstX; x <= lastX; ++x) {
        collision.cellTriangles[static_cast<std::size_t>(z * collision.gridWidth + x)].push_back(
            triangleIndex);
      }
    }
  }
  return result;
}

std::optional<TrackGroundHit> sampleTrackGround(const TrackCollision& collision, const float x,
                                                const float z, const float minimumY,
                                                const float maximumY) noexcept {
  if (minimumY > maximumY || collision.gridWidth <= 0 || collision.gridDepth <= 0) {
    return std::nullopt;
  }
  const int cellX = static_cast<int>(std::floor(x / kTrackHorizontalCellSize));
  const int cellZ = static_cast<int>(std::floor(z / kTrackHorizontalCellSize));
  if (cellX < 0 || cellZ < 0 || cellX >= collision.gridWidth || cellZ >= collision.gridDepth) {
    return std::nullopt;
  }
  const auto& triangles =
      collision.cellTriangles[static_cast<std::size_t>(cellZ * collision.gridWidth + cellX)];
  std::optional<TrackGroundHit> best;
  for (const std::uint32_t triangleIndex : triangles) {
    const std::size_t first = static_cast<std::size_t>(triangleIndex) * 3U;
    if (first + 2U >= collision.indices.size()) {
      continue;
    }
    const gbx::Float3 a = collision.positions[collision.indices[first]];
    const gbx::Float3 b = collision.positions[collision.indices[first + 1U]];
    const gbx::Float3 c = collision.positions[collision.indices[first + 2U]];
    const float denominator = (b.z - c.z) * (a.x - c.x) + (c.x - b.x) * (a.z - c.z);
    if (std::abs(denominator) <= 1.0e-7F) {
      continue;
    }
    const float weightA = ((b.z - c.z) * (x - c.x) + (c.x - b.x) * (z - c.z)) / denominator;
    const float weightB = ((c.z - a.z) * (x - c.x) + (a.x - c.x) * (z - c.z)) / denominator;
    const float weightC = 1.0F - weightA - weightB;
    constexpr float epsilon = -1.0e-5F;
    if (weightA < epsilon || weightB < epsilon || weightC < epsilon) {
      continue;
    }
    const float height = weightA * a.y + weightB * b.y + weightC * c.y;
    if (height < minimumY || height > maximumY || (best.has_value() && height <= best->height)) {
      continue;
    }
    gbx::Float3 normal = normalize(cross(subtract(b, a), subtract(c, a)));
    if (normal.y < 0.0F) {
      normal = {-normal.x, -normal.y, -normal.z};
    }
    best = TrackGroundHit{height, normal,
                          triangleIndex < collision.triangleSurfaceIds.size()
                              ? collision.triangleSurfaceIds[triangleIndex]
                              : static_cast<std::int16_t>(-1),
                          triangleIndex};
  }
  return best;
}

std::optional<TrackWallContact> sampleTrackWallContact(const TrackCollision& collision,
                                                       const gbx::Float3 center,
                                                       const float radius) noexcept {
  if (!std::isfinite(center.x) || !std::isfinite(center.y) || !std::isfinite(center.z) ||
      !std::isfinite(radius) || radius <= 0.0F || collision.gridWidth <= 0 ||
      collision.gridDepth <= 0) {
    return std::nullopt;
  }
  const int firstX =
      std::max(0, static_cast<int>(std::floor((center.x - radius) / kTrackHorizontalCellSize)));
  const int lastX =
      std::min(collision.gridWidth - 1,
               static_cast<int>(std::floor((center.x + radius) / kTrackHorizontalCellSize)));
  const int firstZ =
      std::max(0, static_cast<int>(std::floor((center.z - radius) / kTrackHorizontalCellSize)));
  const int lastZ =
      std::min(collision.gridDepth - 1,
               static_cast<int>(std::floor((center.z + radius) / kTrackHorizontalCellSize)));
  if (firstX > lastX || firstZ > lastZ) {
    return std::nullopt;
  }

  constexpr float kMaximumWallVerticalNormal = 0.65F;
  constexpr float kMinimumHorizontalNormalSquared = 1.0e-6F;
  const float radiusSquared = radius * radius;
  std::optional<TrackWallContact> best;
  for (int z = firstZ; z <= lastZ; ++z) {
    for (int x = firstX; x <= lastX; ++x) {
      const auto& triangles =
          collision.cellTriangles[static_cast<std::size_t>(z * collision.gridWidth + x)];
      for (const std::uint32_t triangleIndex : triangles) {
        const std::size_t first = static_cast<std::size_t>(triangleIndex) * 3U;
        if (first + 2U >= collision.indices.size()) {
          continue;
        }
        const gbx::Float3 a = collision.positions[collision.indices[first]];
        const gbx::Float3 b = collision.positions[collision.indices[first + 1U]];
        const gbx::Float3 c = collision.positions[collision.indices[first + 2U]];
        const gbx::Float3 face = normalize(cross(subtract(b, a), subtract(c, a)));
        if (std::abs(face.y) > kMaximumWallVerticalNormal) {
          continue;
        }
        const gbx::Float3 closest = closestPointOnTriangle(center, a, b, c);
        const gbx::Float3 delta = subtract(center, closest);
        const float distanceSquared = lengthSquared(delta);
        if (distanceSquared >= radiusSquared) {
          continue;
        }
        const float distance = std::sqrt(std::max(distanceSquared, 0.0F));
        gbx::Float3 normal = distance > 1.0e-6F ? multiply(delta, 1.0F / distance) : face;
        const float horizontalSquared = normal.x * normal.x + normal.z * normal.z;
        if (horizontalSquared <= kMinimumHorizontalNormalSquared) {
          continue;
        }
        const float inverseHorizontal = 1.0F / std::sqrt(horizontalSquared);
        normal = {normal.x * inverseHorizontal, 0.0F, normal.z * inverseHorizontal};
        const float penetration = radius - distance;
        if (best.has_value() && penetration <= best->penetration) {
          continue;
        }
        best = TrackWallContact{normal, penetration,
                                triangleIndex < collision.triangleSurfaceIds.size()
                                    ? collision.triangleSurfaceIds[triangleIndex]
                                    : static_cast<std::int16_t>(-1),
                                triangleIndex};
      }
    }
  }
  return best;
}

bool segmentIntersectsTrackTrigger(const TrackTriggerMesh& trigger, const gbx::Float3 from,
                                   const gbx::Float3 to) noexcept {
  if (!std::isfinite(from.x) || !std::isfinite(from.y) || !std::isfinite(from.z) ||
      !std::isfinite(to.x) || !std::isfinite(to.y) || !std::isfinite(to.z)) {
    return false;
  }
  const gbx::Float3 direction = subtract(to, from);
  constexpr float kEpsilon = 1.0e-6F;
  for (std::size_t first = 0; first + 2U < trigger.indices.size(); first += 3U) {
    const std::uint32_t indexA = trigger.indices[first];
    const std::uint32_t indexB = trigger.indices[first + 1U];
    const std::uint32_t indexC = trigger.indices[first + 2U];
    if (indexA >= trigger.positions.size() || indexB >= trigger.positions.size() ||
        indexC >= trigger.positions.size()) {
      continue;
    }
    const gbx::Float3 a = trigger.positions[indexA];
    const gbx::Float3 edge1 = subtract(trigger.positions[indexB], a);
    const gbx::Float3 edge2 = subtract(trigger.positions[indexC], a);
    const gbx::Float3 determinantVector = cross(direction, edge2);
    const float determinant = dot(edge1, determinantVector);
    if (std::abs(determinant) <= kEpsilon) {
      continue;
    }
    const float inverseDeterminant = 1.0F / determinant;
    const gbx::Float3 fromA = subtract(from, a);
    const float u = dot(fromA, determinantVector) * inverseDeterminant;
    if (u < -kEpsilon || u > 1.0F + kEpsilon) {
      continue;
    }
    const gbx::Float3 crossFrom = cross(fromA, edge1);
    const float v = dot(direction, crossFrom) * inverseDeterminant;
    if (v < -kEpsilon || u + v > 1.0F + kEpsilon) {
      continue;
    }
    const float distance = dot(edge2, crossFrom) * inverseDeterminant;
    if (distance >= -kEpsilon && distance <= 1.0F + kEpsilon) {
      return true;
    }
  }
  return false;
}

} // namespace tmnf::content
