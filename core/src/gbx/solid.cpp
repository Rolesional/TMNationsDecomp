#include "tmnf/gbx/solid.hpp"

#include "tmnf/gbx/class_registry.hpp"
#include "tmnf/gbx/document.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace tmnf::gbx {
namespace {

constexpr std::uint32_t kFacade = 0xfacade01U;
constexpr std::uint32_t kSkip = 0x534b4950U;
constexpr std::int32_t kMaximumCount = 1 << 24;

constexpr std::uint32_t kCPlugSolid = 0x09005000U;
constexpr std::uint32_t kCPlugVisual = 0x09006000U;
constexpr std::uint32_t kCPlugSurfaceGeom = 0x0900f000U;
constexpr std::uint32_t kCPlugTreeVisualMip = 0x09015000U;
constexpr std::uint32_t kCPlugVisualSprite = 0x09010000U;
constexpr std::uint32_t kCPlugVisualIndexedTriangles = 0x0901e000U;
constexpr std::uint32_t kCPlugTree = 0x0904f000U;
constexpr std::uint32_t kCPlugIndexBuffer = 0x09057000U;

class Reader final {
public:
  explicit Reader(Input& input) : input_(input) {}

  bool readBytes(const std::span<std::byte> destination) {
    if (input_.position() > input_.size() ||
        destination.size() > input_.size() - input_.position()) {
      return fail("GBX solid read exceeds the declared file size");
    }
    if (!input_.readBytes(destination)) {
      const std::string_view detail = input_.error();
      return fail(detail.empty() ? "unable to read GBX solid" : std::string(detail));
    }
    return true;
  }

  bool readU8(std::uint8_t& value) {
    std::byte encoded{};
    if (!readBytes(std::span(&encoded, 1))) {
      return false;
    }
    value = std::to_integer<std::uint8_t>(encoded);
    return true;
  }

  bool readU16(std::uint16_t& value) {
    std::array<std::byte, 2> bytes{};
    if (!readBytes(bytes)) {
      return false;
    }
    value = static_cast<std::uint16_t>(std::to_integer<unsigned char>(bytes[0])) |
            static_cast<std::uint16_t>(std::to_integer<unsigned char>(bytes[1]) << 8U);
    return true;
  }

  bool readI16(std::int16_t& value) {
    std::uint16_t encoded = 0;
    if (!readU16(encoded)) {
      return false;
    }
    value = std::bit_cast<std::int16_t>(encoded);
    return true;
  }

  bool readU32(std::uint32_t& value) {
    std::array<std::byte, 4> bytes{};
    if (!readBytes(bytes)) {
      return false;
    }
    value = 0;
    for (std::size_t index = 0; index < bytes.size(); ++index) {
      value |= static_cast<std::uint32_t>(std::to_integer<unsigned char>(bytes[index]))
               << (index * 8U);
    }
    return true;
  }

  bool readI32(std::int32_t& value) {
    std::uint32_t encoded = 0;
    if (!readU32(encoded)) {
      return false;
    }
    value = std::bit_cast<std::int32_t>(encoded);
    return true;
  }

  bool readFloat(float& value) {
    std::uint32_t encoded = 0;
    if (!readU32(encoded)) {
      return false;
    }
    value = std::bit_cast<float>(encoded);
    return true;
  }

  bool readBoolean(bool& value) {
    std::uint32_t encoded = 0;
    if (!readU32(encoded) || encoded > 1U) {
      return fail("invalid GBX boolean in solid");
    }
    value = encoded != 0;
    return true;
  }

  bool readCount(std::int32_t& count, const char* const description) {
    if (!readI32(count) || count < 0 || count > kMaximumCount) {
      return fail(std::string("invalid ") + description + " count");
    }
    return true;
  }

  bool skip(const std::size_t count) {
    std::array<std::byte, 4096> buffer{};
    std::size_t remaining = count;
    while (remaining != 0) {
      const std::size_t chunk = std::min(remaining, buffer.size());
      if (!readBytes(std::span(buffer).first(chunk))) {
        return false;
      }
      remaining -= chunk;
    }
    return true;
  }

  bool skipElements(const std::int32_t count, const std::size_t elementSize) {
    if (static_cast<std::size_t>(count) > std::numeric_limits<std::size_t>::max() / elementSize) {
      return fail("GBX solid array size overflow");
    }
    return skip(static_cast<std::size_t>(count) * elementSize);
  }

  bool fail(std::string message) {
    if (error_.empty()) {
      error_ = std::move(message);
    }
    return false;
  }

  [[nodiscard]] std::string takeError() { return std::move(error_); }

private:
  Input& input_;
  std::string error_;
};

std::optional<std::uint32_t> solidClassParent(const std::uint32_t classId) {
  switch (classId) {
  case kCPlugSurfaceGeom:
    return 0x0902b000U;
  case kCPlugTreeVisualMip:
    return kCPlugTree;
  case kCPlugVisualIndexedTriangles:
    return 0x0906a000U;
  default:
    return classParentId(classId);
  }
}

bool knownSolidClass(const std::uint32_t classId) {
  return className(classId).has_value() || classId == kCPlugSurfaceGeom ||
         classId == kCPlugTreeVisualMip || classId == kCPlugVisualIndexedTriangles;
}

std::optional<std::uint32_t> solidDummyWriteSeed(const std::uint32_t classId) {
  if (classId == kCPlugSurfaceGeom || classId == kCPlugTreeVisualMip ||
      classId == kCPlugVisualIndexedTriangles) {
    return solidClassParent(classId);
  }
  return dummyWriteSeedClassId(classId);
}

bool isClassOrDerived(std::uint32_t classId, const std::uint32_t baseClassId) {
  for (std::size_t depth = 0; depth <= classRegistry().size(); ++depth) {
    if (classId == baseClassId) {
      return true;
    }
    const std::optional<std::uint32_t> parent = solidClassParent(classId);
    if (!parent.has_value() || *parent == classId) {
      return false;
    }
    classId = *parent;
  }
  return false;
}

std::string externalPath(const DocumentProbe& document, const ExternalNode& node) {
  std::vector<std::string_view> components;
  std::int32_t directoryIndex = node.directoryIndex;
  while (directoryIndex >= 0 &&
         directoryIndex < static_cast<std::int32_t>(document.directories.size())) {
    const ReferenceDirectory& directory = document.directories[directoryIndex];
    components.push_back(directory.name);
    directoryIndex = directory.parentIndex;
  }
  std::reverse(components.begin(), components.end());
  std::string path;
  for (const std::string_view component : components) {
    if (!path.empty()) {
      path.push_back('\\');
    }
    path.append(component);
  }
  if (!node.name.empty()) {
    if (!path.empty()) {
      path.push_back('\\');
    }
    path.append(node.name);
  }
  return path;
}

class SolidParser final {
public:
  SolidParser(Input& input, const DocumentProbe& document)
      : input_(input), reader_(input), document_(document) {
    NodeRecord root;
    root.classId = document.header.classId;
    records_.emplace(0, std::move(root));
    for (const ExternalNode& external : document.externalNodes) {
      NodeRecord record;
      record.external = true;
      record.externalPath = externalPath(document, external);
      records_.insert_or_assign(external.nodeIndex, std::move(record));
    }
    solid_.externalNodeCount = document.externalNodes.size();
  }

  SolidParseResult run() {
    if (document_.header.classId != kCPlugSolid || !document_.firstChunkId.has_value()) {
      return failed("GBX document is not an uncompressed CPlugSolid");
    }
    if (!parseNode(document_.header.classId, document_.firstChunkId, false, 0)) {
      return failed(reader_.takeError());
    }
    resolveVisualMipLevels();
    if (input_.position() != input_.size()) {
      return failed("CPlugSolid body has trailing logical bytes");
    }
    if (document_.numNodes != static_cast<std::int32_t>(records_.size())) {
      return failed("CPlugSolid parsed node count does not match the document header");
    }
    solid_.parsedNodeCount = records_.size();
    SolidParseResult result;
    result.solid = std::move(solid_);
    return result;
  }

private:
  struct NodeRecord final {
    std::uint32_t classId = 0;
    bool external = false;
    std::optional<std::size_t> visualIndex;
    std::vector<std::int32_t> children;
    std::string name;
    std::string externalPath;
  };

  struct NodeReference final {
    std::int32_t index = -1;
    std::uint32_t classId = 0;
    bool external = false;
    std::optional<std::size_t> visualIndex;
    std::string name;
    std::string externalPath;
  };

  struct VisualMipReference final {
    std::int32_t nodeIndex = -1;
    std::size_t levelIndex = 0;
  };

  SolidParseResult failed(std::string message) {
    SolidParseResult result;
    result.error = std::move(message);
    if (result.error.empty()) {
      result.error = "unable to parse CPlugSolid";
    }
    return result;
  }

  bool initializeNode(const std::uint32_t classId) {
    const std::optional<std::uint32_t> seed = solidDummyWriteSeed(classId);
    if (!seed.has_value()) {
      return reader_.fail("CPlugSolid node has no recovered parent-class seed");
    }
    std::array<std::byte, 4> bytes{};
    for (std::size_t index = 0; index < bytes.size(); ++index) {
      bytes[index] = static_cast<std::byte>((*seed >> (index * 8U)) & 0xffU);
    }
    input_.initialize(bytes);
    return true;
  }

  NodeReference referenceFor(const std::int32_t index, const NodeRecord& record) const {
    return NodeReference{.index = index,
                         .classId = record.classId,
                         .external = record.external,
                         .visualIndex = record.visualIndex,
                         .name = record.name,
                         .externalPath = record.externalPath};
  }

  bool readNodeReference(NodeReference& reference) {
    std::int32_t index = -1;
    if (!reader_.readI32(index)) {
      return false;
    }
    if (index == -1) {
      reference = {};
      return true;
    }
    if (const auto existing = records_.find(index); existing != records_.end()) {
      reference = referenceFor(index, existing->second);
      return true;
    }

    std::uint32_t classId = 0;
    if (!reader_.readU32(classId)) {
      return false;
    }
    if (!knownSolidClass(classId)) {
      return reader_.fail("CPlugSolid contains unknown inline node class 0x" + hex(classId) +
                          " for index " + std::to_string(index) + " at logical offset " +
                          std::to_string(input_.position() - 4U));
    }
    NodeRecord record;
    record.classId = classId;
    records_.emplace(index, std::move(record));
    if (!parseNode(classId, std::nullopt, true, index)) {
      return false;
    }
    reference = referenceFor(index, records_.at(index));
    return true;
  }

  bool parseNode(const std::uint32_t classId, std::optional<std::uint32_t> firstChunk,
                 const bool initialize, const std::optional<std::int32_t> nodeIndex) {
    if (initialize && !initializeNode(classId)) {
      return false;
    }

    std::optional<std::size_t> visualIndex;
    if (isClassOrDerived(classId, kCPlugVisual)) {
      visualIndex = solid_.visuals.size();
      solid_.visuals.emplace_back();
      if (nodeIndex.has_value()) {
        records_.at(*nodeIndex).visualIndex = visualIndex;
      }
    }
    std::string treeName;
    std::string materialReference;
    std::uint32_t treeFlags = 0;
    std::array<float, 12> transform{1.0F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F,
                                    0.0F, 0.0F, 1.0F, 0.0F, 0.0F, 0.0F};
    std::optional<std::size_t> attachedVisual;

    while (true) {
      std::uint32_t chunkId = 0;
      if (firstChunk.has_value()) {
        chunkId = *firstChunk;
        firstChunk.reset();
      } else if (!reader_.readU32(chunkId)) {
        return false;
      }
      if (chunkId == kFacade) {
        break;
      }

      switch (chunkId) {
      case 0x09005000U: {
        std::int32_t ignored = 0;
        if (!reader_.readI32(ignored)) {
          return false;
        }
        break;
      }
      case 0x0900500eU:
        if (!reader_.skip(64)) {
          return false;
        }
        break;
      case 0x09005010U: {
        NodeReference ignored;
        if (!readNodeReference(ignored)) {
          return false;
        }
        break;
      }
      case 0x09005011U: {
        bool ignored = false;
        bool hasExtra = false;
        if (!reader_.readBoolean(ignored) || !reader_.readBoolean(hasExtra)) {
          return false;
        }
        if (hasExtra && !reader_.readBoolean(ignored)) {
          return false;
        }
        NodeReference tree;
        if (!readNodeReference(tree)) {
          return false;
        }
        solid_.rootTreeName = tree.name;
        break;
      }
      case 0x09005012U: {
        std::uint8_t ignored = 0;
        if (!reader_.readU8(ignored)) {
          return false;
        }
        break;
      }
      case 0x0904f006U: {
        std::int32_t deprecVersion = 0;
        std::int32_t count = 0;
        if (!reader_.readI32(deprecVersion) || !reader_.readCount(count, "CPlugTree child")) {
          return false;
        }
        for (std::int32_t index = 0; index < count; ++index) {
          NodeReference child;
          if (!readNodeReference(child)) {
            return false;
          }
          if (nodeIndex.has_value() && child.index >= 0) {
            records_.at(*nodeIndex).children.push_back(child.index);
          }
        }
        break;
      }
      case 0x0904f00dU: {
        std::string ignored;
        if (!readId(treeName) || !readId(ignored)) {
          return false;
        }
        break;
      }
      case 0x0904f011U: {
        NodeReference ignored;
        if (!readNodeReference(ignored)) {
          return false;
        }
        break;
      }
      case 0x0904f016U: {
        NodeReference visual;
        NodeReference shader;
        NodeReference surface;
        NodeReference generator;
        if (!readNodeReference(visual) || !readNodeReference(shader) ||
            !readNodeReference(surface) || !readNodeReference(generator)) {
          return false;
        }
        attachedVisual = visual.visualIndex;
        materialReference = shader.externalPath;
        break;
      }
      case 0x0904f01aU:
        if (!reader_.readU32(treeFlags)) {
          return false;
        }
        if ((treeFlags & 4U) != 0) {
          for (float& value : transform) {
            if (!reader_.readFloat(value)) {
              return false;
            }
          }
        }
        break;
      case 0x09015002U: {
        std::int32_t count = 0;
        if (!reader_.readCount(count, "CPlugTreeVisualMip level")) {
          return false;
        }
        for (std::int32_t index = 0; index < count; ++index) {
          float ignored = 0;
          NodeReference tree;
          if (!reader_.readFloat(ignored) || !readNodeReference(tree)) {
            return false;
          }
          if (tree.index >= 0) {
            visualMipReferences_.push_back(
                VisualMipReference{tree.index, static_cast<std::size_t>(index)});
          }
        }
        break;
      }
      case 0x09006001U: {
        std::string ignored;
        if (!readId(ignored)) {
          return false;
        }
        break;
      }
      case 0x09006004U:
      case 0x0902c002U: {
        NodeReference ignored;
        if (!readNodeReference(ignored)) {
          return false;
        }
        break;
      }
      case 0x09062004U: { // CPlugTreeLight::PlugLight
        NodeReference ignored;
        if (!readNodeReference(ignored)) {
          return false;
        }
        break;
      }
      case 0x09006005U: {
        std::int32_t count = 0;
        if (!reader_.readCount(count, "CPlugVisual subvisual") ||
            !reader_.skipElements(count, 12)) {
          return false;
        }
        break;
      }
      case 0x09006009U: {
        float ignored = 0;
        if (!reader_.readFloat(ignored)) {
          return false;
        }
        break;
      }
      case 0x0900600bU: {
        std::int32_t count = 0;
        if (!reader_.readCount(count, "CPlugVisual split") || !reader_.skipElements(count, 32)) {
          return false;
        }
        break;
      }
      case 0x0900600dU:
      case 0x0900600eU:
        if (!visualIndex.has_value() ||
            !readVisualHeader(solid_.visuals[*visualIndex], chunkId == 0x0900600eU)) {
          return false;
        }
        break;
      case 0x0902c004U:
        if (!visualIndex.has_value() ||
            !readVisualVertices(solid_.visuals[*visualIndex], classId)) {
          return false;
        }
        break;
      case 0x0906a001U: {
        bool hasIndexBuffer = false;
        if (!visualIndex.has_value() || !reader_.readBoolean(hasIndexBuffer)) {
          return false;
        }
        if (hasIndexBuffer && (!initializeNode(kCPlugIndexBuffer) ||
                               !readIndexBuffer(solid_.visuals[*visualIndex]))) {
          return false;
        }
        break;
      }
      case 0x0900c000U:
        if (!readSurface()) {
          return false;
        }
        break;
      case 0x0900f004U:
        if (!readSurfaceGeom()) {
          return false;
        }
        break;
      default:
        if (!skipUnknownChunk(chunkId)) {
          return false;
        }
        break;
      }
    }

    if (isClassOrDerived(classId, kCPlugTree)) {
      if (nodeIndex.has_value() && attachedVisual.has_value()) {
        records_.at(*nodeIndex).visualIndex = attachedVisual;
      }
      if (nodeIndex.has_value()) {
        records_.at(*nodeIndex).name = treeName;
      }
      if (attachedVisual.has_value()) {
        SolidVisual& visual = solid_.visuals[*attachedVisual];
        visual.treeName = treeName;
        visual.materialReference = materialReference;
        visual.treeFlags = treeFlags;
        visual.transform = transform;
      }
    }
    return true;
  }

  bool readVisualHeader(SolidVisual& visual, const bool hasBitmapElements) {
    std::int32_t chunkFlags = 0;
    std::int32_t textureCoordinateCount = 0;
    std::int32_t vertexCount = 0;
    std::int32_t vertexStreamCount = 0;
    if (!reader_.readI32(chunkFlags) ||
        !reader_.readCount(textureCoordinateCount, "texture-coordinate set") ||
        !reader_.readCount(vertexCount, "visual vertex") ||
        !reader_.readCount(vertexStreamCount, "vertex stream")) {
      return false;
    }
    visual.flags = convertVisualFlags(static_cast<std::uint32_t>(chunkFlags));
    visual.declaredVertexCount = static_cast<std::size_t>(vertexCount);
    if (vertexStreamCount != 0) {
      return reader_.fail("CPlugSolid vertex-stream visuals are not yet supported");
    }
    for (std::int32_t index = 0; index < vertexStreamCount; ++index) {
      NodeReference ignored;
      if (!readNodeReference(ignored)) {
        return false;
      }
    }

    visual.textureCoordinates.clear();
    visual.textureCoordinates.reserve(static_cast<std::size_t>(textureCoordinateCount));
    for (std::int32_t setIndex = 0; setIndex < textureCoordinateCount; ++setIndex) {
      std::int32_t version = 0;
      std::int32_t flags = 0;
      if (!reader_.readI32(version) || version < 0 || version > 32) {
        return reader_.fail("invalid CPlugVisual texture-coordinate version");
      }
      if (version >= 3) {
        std::int32_t actualCount = 0;
        if (!reader_.readI32(actualCount) || actualCount != vertexCount ||
            !reader_.readI32(flags) || (static_cast<unsigned int>(flags) & 0xffU) > 2U) {
          return reader_.fail("invalid CPlugVisual texture-coordinate metadata");
        }
      }
      std::vector<Float2> coordinates(static_cast<std::size_t>(vertexCount));
      for (Float2& coordinate : coordinates) {
        if (!reader_.readFloat(coordinate.x) || !reader_.readFloat(coordinate.y)) {
          return false;
        }
        if (version >= 1 && version < 3 && !reader_.skip(version >= 2 ? 8U : 4U)) {
          return false;
        }
      }
      if (version >= 3) {
        const std::size_t extraCount =
            (static_cast<std::size_t>(vertexCount) * static_cast<std::uint32_t>(flags)) & 0xffU;
        if (!reader_.skip(extraCount * sizeof(float))) {
          return false;
        }
      }
      visual.textureCoordinates.push_back(std::move(coordinates));
    }
    if ((visual.flags & 7U) != 0) {
      return reader_.fail("skinned CPlugVisual data is not yet supported");
    }
    for (float& value : visual.boundingBox) {
      if (!reader_.readFloat(value)) {
        return false;
      }
    }
    if (hasBitmapElements) {
      std::int32_t count = 0;
      if (!reader_.readCount(count, "bitmap-pack element") || !reader_.skipElements(count, 20)) {
        return false;
      }
    }
    visual.positions.reserve(static_cast<std::size_t>(vertexCount));
    visual.normals.reserve(static_cast<std::size_t>(vertexCount));
    return true;
  }

  bool readVisualVertices(SolidVisual& visual, const std::uint32_t classId) {
    const std::size_t vertexCount = visual.declaredVertexCount;
    const bool hasNormal = (visual.flags & (1U << 22U)) == 0 || (visual.flags & (1U << 7U)) != 0;
    const bool hasColor = (visual.flags & (1U << 22U)) == 0 || (visual.flags & (1U << 8U)) != 0;
    const bool packedNormal = (visual.flags & (1U << 20U)) != 0;
    const bool packedColor = (visual.flags & (1U << 21U)) != 0;
    const bool sprite = isClassOrDerived(classId, kCPlugVisualSprite);
    visual.positions.clear();
    visual.normals.clear();
    for (std::size_t index = 0; index < vertexCount; ++index) {
      Float3 position;
      if (!reader_.readFloat(position.x) || !reader_.readFloat(position.y) ||
          !reader_.readFloat(position.z)) {
        return false;
      }
      visual.positions.push_back(position);
      if (hasNormal) {
        Float3 normal;
        if (packedNormal) {
          std::uint32_t encoded = 0;
          if (!reader_.readU32(encoded)) {
            return false;
          }
          normal = decodePackedNormal(encoded);
        } else if (!reader_.readFloat(normal.x) || !reader_.readFloat(normal.y) ||
                   !reader_.readFloat(normal.z)) {
          return false;
        }
        visual.normals.push_back(normal);
      }
      if (hasColor && !reader_.skip(packedColor ? 4U : 16U)) {
        return false;
      }
      if (sprite && !reader_.skip(8)) {
        return false;
      }
    }

    const std::size_t tangentSize = ((~static_cast<std::uint8_t>(visual.flags >> 17U) & 8U) | 4U);
    for (int tangentSet = 0; tangentSet < 2; ++tangentSet) {
      std::int32_t count = 0;
      if (!reader_.readCount(count, "visual tangent") ||
          (count != 0 && static_cast<std::size_t>(count) != vertexCount) ||
          !reader_.skipElements(count, tangentSize)) {
        return reader_.fail("invalid CPlugVisual tangent data");
      }
    }
    return true;
  }

  bool readIndexBuffer(SolidVisual& visual) {
    while (true) {
      std::uint32_t chunkId = 0;
      if (!reader_.readU32(chunkId)) {
        return false;
      }
      if (chunkId == kFacade) {
        return true;
      }
      if (chunkId != 0x09057000U && chunkId != 0x09057001U) {
        if (!skipUnknownChunk(chunkId)) {
          return false;
        }
        continue;
      }
      std::int32_t flags = 0;
      std::int32_t count = 0;
      if (!reader_.readI32(flags) || !reader_.readCount(count, "index")) {
        return false;
      }
      visual.indices.resize(static_cast<std::size_t>(count));
      std::int32_t current = 0;
      for (std::uint32_t& index : visual.indices) {
        if (chunkId == 0x09057000U) {
          std::uint16_t encoded = 0;
          if (!reader_.readU16(encoded)) {
            return false;
          }
          index = encoded;
        } else {
          std::int16_t delta = 0;
          if (!reader_.readI16(delta)) {
            return false;
          }
          current += delta;
          if (current < 0) {
            return reader_.fail("negative delta-coded CPlugIndexBuffer index");
          }
          index = static_cast<std::uint32_t>(current);
        }
      }
    }
  }

  bool readSurface() {
    NodeReference geometry;
    std::int32_t materialCount = 0;
    if (!readNodeReference(geometry) || !reader_.readCount(materialCount, "surface material")) {
      return false;
    }
    for (std::int32_t index = 0; index < materialCount; ++index) {
      bool hasMaterial = false;
      if (!reader_.readBoolean(hasMaterial)) {
        return false;
      }
      if (hasMaterial) {
        NodeReference material;
        if (!readNodeReference(material)) {
          return false;
        }
        solid_.surfaceMaterials.push_back(-1);
      } else {
        std::int16_t surfaceId = 0;
        if (!reader_.readI16(surfaceId)) {
          return false;
        }
        solid_.surfaceMaterials.push_back(surfaceId);
      }
    }
    return true;
  }

  bool readSurfaceGeom() {
    std::string ignored;
    std::array<float, 6> box{};
    if (!readId(ignored)) {
      return false;
    }
    for (float& value : box) {
      if (!reader_.readFloat(value)) {
        return false;
      }
    }
    const std::uint32_t encodedSeed = std::bit_cast<std::uint32_t>(box[0] - box[3]);
    std::array<std::byte, 4> seed{};
    for (std::size_t index = 0; index < seed.size(); ++index) {
      seed[index] = static_cast<std::byte>((encodedSeed >> (index * 8U)) & 0xffU);
    }
    input_.initialize(seed);
    const std::size_t firstShape = solid_.collisionShapes.size();
    if (!readSurfaceShape(0)) {
      return false;
    }
    std::int16_t surfaceId = 0;
    if (!reader_.readI16(surfaceId)) {
      return false;
    }
    for (std::size_t index = firstShape; index < solid_.collisionShapes.size(); ++index) {
      if (solid_.collisionShapes[index].surfaceId < 0) {
        solid_.collisionShapes[index].surfaceId = surfaceId;
      }
    }
    return true;
  }

  bool readSurfaceShape(const std::int32_t surfVersion) {
    std::int32_t type = 0;
    if (!reader_.readI32(type)) {
      return false;
    }
    switch (type) {
    case 0: {
      SolidCollisionShape shape;
      shape.type = SolidCollisionType::Sphere;
      float radius = 0.0F;
      if (!reader_.readFloat(radius)) {
        return false;
      }
      shape.minimum = {-radius, -radius, -radius};
      shape.maximum = {radius, radius, radius};
      if (surfVersion >= 1 && !reader_.readI16(shape.surfaceId)) {
        return false;
      }
      solid_.collisionShapes.push_back(std::move(shape));
      break;
    }
    case 1: {
      SolidCollisionShape shape;
      shape.type = SolidCollisionType::Ellipsoid;
      Float3 radius;
      if (!reader_.readFloat(radius.x) || !reader_.readFloat(radius.y) ||
          !reader_.readFloat(radius.z)) {
        return false;
      }
      shape.minimum = {-radius.x, -radius.y, -radius.z};
      shape.maximum = radius;
      if (surfVersion >= 1 && !reader_.readI16(shape.surfaceId)) {
        return false;
      }
      solid_.collisionShapes.push_back(std::move(shape));
      break;
    }
    case 6: {
      SolidCollisionShape shape;
      shape.type = SolidCollisionType::Box;
      if (!reader_.readFloat(shape.minimum.x) || !reader_.readFloat(shape.minimum.y) ||
          !reader_.readFloat(shape.minimum.z) || !reader_.readFloat(shape.maximum.x) ||
          !reader_.readFloat(shape.maximum.y) || !reader_.readFloat(shape.maximum.z)) {
        return false;
      }
      if (surfVersion >= 1 && !reader_.readI16(shape.surfaceId)) {
        return false;
      }
      solid_.collisionShapes.push_back(std::move(shape));
      break;
    }
    case 7:
      if (!readSurfaceMesh()) {
        return false;
      }
      break;
    case 13: {
      std::int32_t count = 0;
      if (!reader_.readCount(count, "compound surface")) {
        return false;
      }
      std::vector<std::pair<std::size_t, std::size_t>> childRanges;
      childRanges.reserve(static_cast<std::size_t>(count));
      for (std::int32_t index = 0; index < count; ++index) {
        const std::size_t begin = solid_.collisionShapes.size();
        if (!readSurfaceShape(surfVersion)) {
          return false;
        }
        childRanges.emplace_back(begin, solid_.collisionShapes.size());
      }
      for (const auto [begin, end] : childRanges) {
        std::array<float, 12> transform{};
        for (float& value : transform) {
          if (!reader_.readFloat(value)) {
            return false;
          }
        }
        for (std::size_t shape = begin; shape < end; ++shape) {
          solid_.collisionShapes[shape].transform =
              multiplyIso4(transform, solid_.collisionShapes[shape].transform);
        }
      }
      if (surfVersion >= 1) {
        std::int32_t jointCount = 0;
        if (!reader_.readCount(jointCount, "surface joint") ||
            !reader_.skipElements(jointCount, 2)) {
          return false;
        }
      }
      break;
    }
    default:
      return reader_.fail("unsupported CPlugSurface shape type");
    }
    return surfVersion < 2 || reader_.skip(12);
  }

  bool readSurfaceMesh() {
    std::int32_t version = 0;
    if (!reader_.readI32(version)) {
      return false;
    }
    SolidCollisionShape shape;
    shape.type = SolidCollisionType::Mesh;
    shape.archiveVersion = version;
    std::int32_t count = 0;
    if (!reader_.readCount(count, "surface vertex")) {
      return false;
    }
    shape.positions.resize(static_cast<std::size_t>(count));
    shape.minimum = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                     std::numeric_limits<float>::max()};
    shape.maximum = {std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(),
                     std::numeric_limits<float>::lowest()};
    for (Float3& vertex : shape.positions) {
      if (!reader_.readFloat(vertex.x) || !reader_.readFloat(vertex.y) ||
          !reader_.readFloat(vertex.z)) {
        return false;
      }
      shape.minimum.x = std::min(shape.minimum.x, vertex.x);
      shape.minimum.y = std::min(shape.minimum.y, vertex.y);
      shape.minimum.z = std::min(shape.minimum.z, vertex.z);
      shape.maximum.x = std::max(shape.maximum.x, vertex.x);
      shape.maximum.y = std::max(shape.maximum.y, vertex.y);
      shape.maximum.z = std::max(shape.maximum.z, vertex.z);
    }
    if (shape.positions.empty()) {
      shape.minimum = {};
      shape.maximum = {};
    }
    if (version >= 1 && version <= 3) {
      if (!readCookedSurfaceTriangles(shape, count)) {
        return false;
      }
      std::int32_t octreeVersion = 0;
      if (!reader_.readI32(octreeVersion)) {
        return false;
      }
      if (octreeVersion == 1) {
        std::int32_t deprecVersion = 0;
        if (!reader_.readI32(deprecVersion)) {
          return false;
        }
      }
      if (!reader_.readCount(count, "surface octree cell") ||
          !reader_.skipElements(count, 32)) {
        return false;
      }
      solid_.collisionShapes.push_back(std::move(shape));
      return true;
    }
    if (version == 5) {
      if (!readCookedSurfaceTriangles(shape, count)) {
        return false;
      }
      std::int32_t ignored = 0;
      if (!reader_.readI32(ignored) || !reader_.readCount(count, "surface AABB cell") ||
          !reader_.skipElements(count, 28)) {
        return false;
      }
      solid_.collisionShapes.push_back(std::move(shape));
      return true;
    }
    if (version == 6 || version == 7) {
      if (!reader_.readCount(count, "surface triangle")) {
        return false;
      }
      for (std::int32_t triangle = 0; triangle < count; ++triangle) {
        std::array<std::int32_t, 3> triangleIndices{};
        std::uint8_t ignoredByte = 0;
        std::int16_t surfaceIndex = 0;
        if (!reader_.readI32(triangleIndices[0]) || !reader_.readI32(triangleIndices[1]) ||
            !reader_.readI32(triangleIndices[2]) || !reader_.readU8(ignoredByte) ||
            !reader_.readU8(ignoredByte) || !reader_.readI16(surfaceIndex) ||
            !appendSurfaceTriangle(shape, triangleIndices, surfaceIndex)) {
          return false;
        }
      }
      solid_.collisionShapes.push_back(std::move(shape));
      return true;
    }
    return reader_.fail("unsupported CPlugSurface mesh version");
  }

  bool readCookedSurfaceTriangles(SolidCollisionShape& shape, std::int32_t& count) {
    if (!reader_.readCount(count, "cooked surface triangle")) {
      return false;
    }
    for (std::int32_t triangle = 0; triangle < count; ++triangle) {
      std::array<std::int32_t, 3> triangleIndices{};
      std::int16_t surfaceIndex = 0;
      std::uint8_t ignored = 0;
      if (!reader_.skip(16) || !reader_.readI32(triangleIndices[0]) ||
          !reader_.readI32(triangleIndices[1]) || !reader_.readI32(triangleIndices[2]) ||
          !reader_.readI16(surfaceIndex) || !reader_.readU8(ignored) ||
          !reader_.readU8(ignored) ||
          !appendSurfaceTriangle(shape, triangleIndices, surfaceIndex)) {
        return false;
      }
    }
    return true;
  }

  bool appendSurfaceTriangle(SolidCollisionShape& shape,
                             const std::array<std::int32_t, 3>& triangle,
                             const std::int16_t surfaceIndex) {
    for (const std::int32_t index : triangle) {
      if (index < 0 || static_cast<std::size_t>(index) >= shape.positions.size()) {
        return reader_.fail("CPlugSurface triangle index is out of bounds");
      }
      shape.indices.push_back(static_cast<std::uint32_t>(index));
    }
    shape.triangleSurfaceIndices.push_back(surfaceIndex);
    return true;
  }

  static std::array<float, 12> multiplyIso4(const std::array<float, 12>& parent,
                                            const std::array<float, 12>& child) {
    std::array<float, 12> result{};
    for (std::size_t row = 0; row < 3; ++row) {
      for (std::size_t column = 0; column < 3; ++column) {
        for (std::size_t inner = 0; inner < 3; ++inner) {
          result[row * 3U + column] +=
              parent[row * 3U + inner] * child[inner * 3U + column];
        }
      }
      result[9U + row] = parent[row * 3U] * child[9U] +
                         parent[row * 3U + 1U] * child[10U] +
                         parent[row * 3U + 2U] * child[11U] + parent[9U + row];
    }
    return result;
  }

  bool readId(std::string& value) {
    if (!idVersion_.has_value()) {
      std::int32_t version = 0;
      if (!reader_.readI32(version) || version < 3) {
        return reader_.fail("unsupported GBX ID version in CPlugSolid");
      }
      idVersion_ = version;
    }
    std::uint32_t index = 0;
    if (!reader_.readU32(index)) {
      return false;
    }
    if (index == std::numeric_limits<std::uint32_t>::max()) {
      value.clear();
      return true;
    }
    if ((index & 0xc0000000U) != 0x40000000U && (index & 0xc0000000U) != 0x80000000U) {
      value = std::to_string(index);
      return true;
    }
    if ((index & 0x3fffffffU) != 0) {
      const auto existing = ids_.find(index);
      value = existing == ids_.end() ? std::string{} : existing->second;
      return true;
    }
    std::int32_t length = 0;
    if (!reader_.readCount(length, "ID string")) {
      return false;
    }
    value.resize(static_cast<std::size_t>(length));
    if (!reader_.readBytes(std::span(reinterpret_cast<std::byte*>(value.data()), value.size()))) {
      return false;
    }
    ids_.insert_or_assign(index + static_cast<std::uint32_t>(ids_.size()) + 1U, value);
    return true;
  }

  bool skipUnknownChunk(const std::uint32_t chunkId) {
    std::uint32_t marker = 0;
    std::int32_t size = 0;
    if (!reader_.readU32(marker) || marker != kSkip ||
        !reader_.readCount(size, "skippable chunk")) {
      return reader_.fail("unknown unskippable CPlugSolid chunk 0x" + hex(chunkId));
    }
    return reader_.skip(static_cast<std::size_t>(size));
  }

  void resolveVisualMipLevels() {
    std::unordered_set<std::size_t> mipVisuals;
    std::unordered_set<std::size_t> primaryVisuals;
    for (const VisualMipReference& reference : visualMipReferences_) {
      std::unordered_set<std::int32_t> visited;
      collectVisuals(reference.nodeIndex, mipVisuals, visited);
    }
    for (const VisualMipReference& reference : visualMipReferences_) {
      if (reference.levelIndex != 0U) {
        continue;
      }
      std::unordered_set<std::int32_t> visited;
      collectVisuals(reference.nodeIndex, primaryVisuals, visited);
    }
    for (const std::size_t visual : mipVisuals) {
      solid_.visuals[visual].renderEnabled = primaryVisuals.contains(visual);
    }
  }

  void collectVisuals(const std::int32_t nodeIndex, std::unordered_set<std::size_t>& visuals,
                      std::unordered_set<std::int32_t>& visited) const {
    if (!visited.insert(nodeIndex).second) {
      return;
    }
    const auto node = records_.find(nodeIndex);
    if (node == records_.end()) {
      return;
    }
    if (node->second.visualIndex.has_value()) {
      visuals.insert(*node->second.visualIndex);
    }
    for (const std::int32_t child : node->second.children) {
      collectVisuals(child, visuals, visited);
    }
  }

  static std::string hex(const std::uint32_t value) {
    constexpr std::string_view digits = "0123456789ABCDEF";
    std::string result(8, '0');
    for (std::size_t index = 0; index < result.size(); ++index) {
      result[result.size() - index - 1U] = digits[(value >> (index * 4U)) & 0x0fU];
    }
    return result;
  }

  static std::uint32_t convertVisualFlags(const std::uint32_t chunkFlags) {
    std::uint32_t flags = chunkFlags & 15U;
    flags |= (chunkFlags << 1U) & 0x20U;
    flags |= (chunkFlags << 2U) & 0x80U;
    flags |= (chunkFlags << 2U) & 0x100U;
    flags |= (chunkFlags << 13U) & 0x100000U;
    flags |= (chunkFlags << 13U) & 0x200000U;
    flags |= (chunkFlags << 13U) & 0x400000U;
    return flags;
  }

  static Float3 decodePackedNormal(const std::uint32_t encoded) {
    const auto decode = [](const std::uint32_t value) {
      const std::int32_t signedValue = (value & 0x200U) != 0
                                           ? static_cast<std::int32_t>(value | 0xfffffc00U)
                                           : static_cast<std::int32_t>(value);
      return static_cast<float>(signedValue) / 511.0F;
    };
    return Float3{decode(encoded & 0x3ffU), decode((encoded >> 10U) & 0x3ffU),
                  decode((encoded >> 20U) & 0x3ffU)};
  }

  Input& input_;
  Reader reader_;
  const DocumentProbe& document_;
  SolidModel solid_;
  std::unordered_map<std::int32_t, NodeRecord> records_;
  std::vector<VisualMipReference> visualMipReferences_;
  std::optional<std::int32_t> idVersion_;
  std::unordered_map<std::uint32_t, std::string> ids_;
};

} // namespace

SolidParseResult parseSolid(Input& input) {
  const DocumentProbeResult document = probeDocument(input, false);
  if (!document) {
    SolidParseResult result;
    result.error = document.error;
    return result;
  }
  SolidParser parser(input, document.document);
  return parser.run();
}

} // namespace tmnf::gbx
