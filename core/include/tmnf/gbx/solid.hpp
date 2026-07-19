#pragma once

#include "tmnf/gbx/input.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace tmnf::gbx {

struct Float2 final {
  float x = 0.0F;
  float y = 0.0F;
};

struct Float3 final {
  float x = 0.0F;
  float y = 0.0F;
  float z = 0.0F;
};

struct SolidVisual final {
  std::string treeName;
  std::string materialReference;
  std::uint32_t flags = 0;
  std::uint32_t treeFlags = 0;
  bool renderEnabled = true;
  std::size_t declaredVertexCount = 0;
  std::array<float, 12> transform{1.0F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F,
                                  0.0F, 0.0F, 1.0F, 0.0F, 0.0F, 0.0F};
  std::array<float, 6> boundingBox{};
  std::vector<Float3> positions;
  std::vector<Float3> normals;
  std::vector<std::vector<Float2>> textureCoordinates;
  std::vector<std::uint32_t> indices;
};

enum class SolidCollisionType {
  Sphere,
  Ellipsoid,
  Box,
  Mesh,
};

struct SolidCollisionShape final {
  std::string treeName;
  SolidCollisionType type = SolidCollisionType::Mesh;
  std::int32_t archiveVersion = 0;
  std::array<float, 12> transform{1.0F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F,
                                  0.0F, 0.0F, 1.0F, 0.0F, 0.0F, 0.0F};
  Float3 minimum;
  Float3 maximum;
  std::int16_t surfaceId = -1;
  std::vector<Float3> positions;
  std::vector<std::uint32_t> indices;
  std::vector<std::int16_t> triangleSurfaceIndices;
  // Local material indices in this shape address this [offset,count] range in
  // SolidModel::surfaceMaterials. Synthetic fixtures may leave it unset to
  // address the complete table.
  std::optional<std::array<std::size_t, 2>> surfaceMaterialRange;
};

struct SolidTreeTransform final {
  std::string name;
  std::array<float, 12> transform{1.0F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F,
                                  0.0F, 0.0F, 1.0F, 0.0F, 0.0F, 0.0F};
};

struct SolidModel final {
  std::string rootTreeName;
  std::vector<SolidVisual> visuals;
  std::vector<SolidCollisionShape> collisionShapes;
  std::vector<SolidTreeTransform> treeTransforms;
  std::vector<std::int16_t> surfaceMaterials;
  // One external CPlugMaterial path per surfaceMaterials entry; empty when the
  // CPlugSurface stores an inline numeric material ID.
  std::vector<std::string> surfaceMaterialReferences;
  std::size_t parsedNodeCount = 0;
  std::size_t externalNodeCount = 0;
};

struct SolidParseResult final {
  SolidModel solid;
  std::string error;

  [[nodiscard]] explicit operator bool() const noexcept { return error.empty(); }
};

// Parses the TMF CPlugSolid/CPlugTree/CPlugVisual graph used by packed Stadium meshes.
// The same calls that create nested nodes are forwarded to Input::initialize so encrypted
// NadeoPak streams receive their original parser-timed dummy-write seeds.
[[nodiscard]] SolidParseResult parseSolid(Input& input);

} // namespace tmnf::gbx
