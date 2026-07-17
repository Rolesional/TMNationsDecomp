#include "tmnf/crypto/md5.hpp"
#include "tmnf/gbx/solid.hpp"
#include "tmnf/pak/vfs.hpp"

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <span>
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

template <typename T> std::string vectorMd5(const std::vector<T>& values) {
  const std::span bytes(reinterpret_cast<const std::byte*>(values.data()),
                        values.size() * sizeof(T));
  return tmnf::crypto::toHex(tmnf::crypto::md5(bytes));
}

std::string_view collisionTypeName(const tmnf::gbx::SolidCollisionType type) {
  switch (type) {
  case tmnf::gbx::SolidCollisionType::Sphere:
    return "sphere";
  case tmnf::gbx::SolidCollisionType::Ellipsoid:
    return "ellipsoid";
  case tmnf::gbx::SolidCollisionType::Box:
    return "box";
  default:
    return "mesh";
  }
}

} // namespace

int main(const int argc, const char* const* argv) {
  if (argc != 4 && argc != 5) {
    std::cerr << "usage: archive_solid_probe <file.pak> <packlist.dat> <logical-entry> "
                 "[identifier]\n";
    return 2;
  }
  const std::filesystem::path archivePath(argv[1]);
  std::vector<std::byte> archiveBytes;
  std::vector<std::byte> packListBytes;
  if (!readFile(archivePath, archiveBytes) || !readFile(argv[2], packListBytes)) {
    std::cerr << "unable to read archive or packlist\n";
    return 2;
  }

  tmnf::pak::VirtualFileSystem vfs;
  const tmnf::pak::VfsOperationResult initialized = vfs.initialize(packListBytes);
  if (!initialized) {
    std::cerr << initialized.error << '\n';
    return 1;
  }
  const std::string identifier = argc == 5 ? argv[4] : archivePath.stem().string();
  const tmnf::pak::VfsOperationResult mounted = vfs.mount(identifier, std::move(archiveBytes));
  if (!mounted) {
    std::cerr << mounted.error << '\n';
    return 1;
  }
  tmnf::pak::VfsOpenResult opened = vfs.openFile(argv[3]);
  if (!opened) {
    std::cerr << opened.error << '\n';
    return 1;
  }
  tmnf::gbx::SolidParseResult parsed = tmnf::gbx::parseSolid(*opened.input);
  if (!parsed) {
    std::cerr << "unable to parse packed solid: " << parsed.error << '\n';
    return 1;
  }

  const tmnf::gbx::SolidModel& solid = parsed.solid;
  std::size_t vertexCount = 0;
  std::size_t indexCount = 0;
  std::size_t collisionVertexCount = 0;
  std::size_t collisionIndexCount = 0;
  tmnf::gbx::Float3 minimum{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                            std::numeric_limits<float>::max()};
  tmnf::gbx::Float3 maximum{std::numeric_limits<float>::lowest(),
                            std::numeric_limits<float>::lowest(),
                            std::numeric_limits<float>::lowest()};
  for (const tmnf::gbx::SolidVisual& visual : solid.visuals) {
    vertexCount += visual.positions.size();
    indexCount += visual.indices.size();
    for (const tmnf::gbx::Float3 position : visual.positions) {
      const auto& matrix = visual.transform;
      const tmnf::gbx::Float3 transformed{
          matrix[0] * position.x + matrix[1] * position.y + matrix[2] * position.z + matrix[9],
          matrix[3] * position.x + matrix[4] * position.y + matrix[5] * position.z + matrix[10],
          matrix[6] * position.x + matrix[7] * position.y + matrix[8] * position.z + matrix[11],
      };
      minimum.x = std::min(minimum.x, transformed.x);
      minimum.y = std::min(minimum.y, transformed.y);
      minimum.z = std::min(minimum.z, transformed.z);
      maximum.x = std::max(maximum.x, transformed.x);
      maximum.y = std::max(maximum.y, transformed.y);
      maximum.z = std::max(maximum.z, transformed.z);
    }
  }
  for (const tmnf::gbx::SolidCollisionShape& shape : solid.collisionShapes) {
    collisionVertexCount += shape.positions.size();
    collisionIndexCount += shape.indices.size();
  }
  std::cout << "{\n"
            << "  \"storedPath\": \"" << jsonEscape(opened.storedPath) << "\",\n"
            << "  \"rootTreeName\": \"" << jsonEscape(solid.rootTreeName) << "\",\n"
            << "  \"nodeCount\": " << solid.parsedNodeCount << ",\n"
            << "  \"externalNodeCount\": " << solid.externalNodeCount << ",\n"
            << "  \"visualCount\": " << solid.visuals.size() << ",\n"
            << "  \"vertexCount\": " << vertexCount << ",\n"
            << "  \"indexCount\": " << indexCount << ",\n"
            << "  \"collisionShapeCount\": " << solid.collisionShapes.size() << ",\n"
            << "  \"collisionVertexCount\": " << collisionVertexCount << ",\n"
            << "  \"collisionIndexCount\": " << collisionIndexCount << ",\n"
            << "  \"bounds\": {\"minimum\": [" << minimum.x << ", " << minimum.y << ", "
            << minimum.z << "], \"maximum\": [" << maximum.x << ", " << maximum.y << ", "
            << maximum.z << "]},\n"
            << "  \"collisionShapes\": [\n";
  for (std::size_t index = 0; index < solid.collisionShapes.size(); ++index) {
    const tmnf::gbx::SolidCollisionShape& shape = solid.collisionShapes[index];
    std::cout << "    {\"type\": \"" << collisionTypeName(shape.type)
              << "\", \"version\": " << shape.archiveVersion
              << ", \"surfaceId\": " << shape.surfaceId
              << ", \"vertices\": " << shape.positions.size()
              << ", \"indices\": " << shape.indices.size()
              << ", \"triangleSurfaces\": " << shape.triangleSurfaceIndices.size()
              << ", \"minimum\": [" << shape.minimum.x << ", " << shape.minimum.y << ", "
              << shape.minimum.z << "], \"maximum\": [" << shape.maximum.x << ", "
              << shape.maximum.y << ", " << shape.maximum.z << "]"
              << ", \"positionMd5\": \"" << vectorMd5(shape.positions)
              << "\", \"indexMd5\": \"" << vectorMd5(shape.indices) << "\"}"
              << (index + 1U == solid.collisionShapes.size() ? "" : ",") << '\n';
  }
  std::cout << "  ],\n"
            << "  \"visuals\": [\n";
  for (std::size_t index = 0; index < solid.visuals.size(); ++index) {
    const tmnf::gbx::SolidVisual& visual = solid.visuals[index];
    std::cout << "    {\"treeName\": \"" << jsonEscape(visual.treeName) << "\", \"material\": \""
              << jsonEscape(visual.materialReference) << "\", \"flags\": " << visual.flags
              << ", \"vertices\": " << visual.positions.size()
              << ", \"normals\": " << visual.normals.size()
              << ", \"renderEnabled\": " << (visual.renderEnabled ? "true" : "false")
              << ", \"texCoordSets\": " << visual.textureCoordinates.size()
              << ", \"indices\": " << visual.indices.size() << ", \"positionMd5\": \""
              << vectorMd5(visual.positions) << "\", \"indexMd5\": \"" << vectorMd5(visual.indices)
              << "\"";
    if (visual.positions.size() <= 8U) {
      std::cout << ", \"vertexSamples\": [";
      for (std::size_t vertexIndex = 0; vertexIndex < visual.positions.size(); ++vertexIndex) {
        const tmnf::gbx::Float3 position = visual.positions[vertexIndex];
        const tmnf::gbx::Float3 normal = vertexIndex < visual.normals.size()
                                                ? visual.normals[vertexIndex]
                                                : tmnf::gbx::Float3{};
        const tmnf::gbx::Float2 texCoord =
            !visual.textureCoordinates.empty() &&
                    vertexIndex < visual.textureCoordinates.front().size()
                ? visual.textureCoordinates.front()[vertexIndex]
                : tmnf::gbx::Float2{};
        std::cout << "{\"position\":[" << position.x << ',' << position.y << ',' << position.z
                  << "],\"normal\":[" << normal.x << ',' << normal.y << ',' << normal.z
                  << "],\"uv\":[" << texCoord.x << ',' << texCoord.y << "]}"
                  << (vertexIndex + 1U == visual.positions.size() ? "" : ",");
      }
      std::cout << "], \"indexSamples\": [";
      for (std::size_t sampleIndex = 0; sampleIndex < visual.indices.size(); ++sampleIndex) {
        std::cout << visual.indices[sampleIndex]
                  << (sampleIndex + 1U == visual.indices.size() ? "" : ",");
      }
      std::cout << ']';
    }
    std::cout << "}" << (index + 1U == solid.visuals.size() ? "" : ",") << '\n';
  }
  std::cout << "  ]\n}\n";
  return 0;
}
