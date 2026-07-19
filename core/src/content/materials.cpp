#include "tmnf/content/materials.hpp"

#include "tmnf/gbx/document.hpp"
#include "tmnf/gbx/span_input.hpp"
#include "tmnf/image/dds.hpp"
#include "tmnf/image/tga.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cctype>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

namespace tmnf::content {
namespace {

SolidMaterialResolveResult failed(std::string message) {
  SolidMaterialResolveResult result;
  result.error = std::move(message);
  return result;
}

SolidMaterialBatchResolveResult batchFailed(std::string message) {
  SolidMaterialBatchResolveResult result;
  result.error = std::move(message);
  return result;
}

SolidSurfaceMaterialResolveResult surfaceFailed(std::string message) {
  SolidSurfaceMaterialResolveResult result;
  result.error = std::move(message);
  return result;
}

std::string lowercase(const std::string_view value) {
  std::string result(value);
  std::transform(result.begin(), result.end(), result.begin(), [](const unsigned char character) {
    return static_cast<char>(std::tolower(character));
  });
  return result;
}

bool endsWith(const std::string_view value, const std::string_view suffix) {
  return value.size() >= suffix.size() && value.substr(value.size() - suffix.size()) == suffix;
}

std::string materialDocumentPath(const std::string_view reference) {
  const std::string normalized = pak::normalizeArchivePath(reference);
  if (normalized.starts_with("media\\") || normalized.find("\\media\\") != std::string::npos) {
    return std::string(reference);
  }
  if (normalized.starts_with("material\\")) {
    return "Media\\" + std::string(reference);
  }
  return "Media\\Material\\" + std::string(reference);
}

struct MaterialSurfaceIdResult final {
  std::int16_t surfaceId = -1;
  std::string error;

  explicit operator bool() const noexcept { return error.empty(); }
};

MaterialSurfaceIdResult readMaterialSurfaceId(const pak::VirtualFileSystem& vfs,
                                              const std::string_view reference) {
  constexpr std::uint32_t kCPlugMaterial = 0x09079000U;
  constexpr std::array<std::byte, 4> kSurfaceIdChunk{
      std::byte{0x0e}, std::byte{0x90}, std::byte{0x07}, std::byte{0x09}};
  const std::string path = materialDocumentPath(reference);
  pak::VfsReadResult bytes = vfs.readFile(path);
  if (!bytes) {
    return {.error = "unable to read " + path + ": " + bytes.error};
  }
  gbx::SpanInput input(bytes.data);
  const gbx::DocumentProbeResult probed = gbx::probeDocument(input, false);
  if (!probed) {
    return {.error = "unable to probe " + path + ": " + probed.error};
  }
  if (probed.document.header.classId != kCPlugMaterial ||
      probed.document.header.bodyCompression != 'U') {
    return {.error = "surface material is not an uncompressed CPlugMaterial: " + path};
  }

  std::optional<std::int16_t> surfaceId;
  const std::span<const std::byte> data(bytes.data);
  for (std::size_t offset = probed.document.bodyDataOffset;
       offset + kSurfaceIdChunk.size() + sizeof(std::int16_t) <= data.size(); ++offset) {
    if (!std::equal(kSurfaceIdChunk.begin(), kSurfaceIdChunk.end(), data.begin() + offset)) {
      continue;
    }
    const std::uint16_t encoded =
        static_cast<std::uint16_t>(std::to_integer<std::uint8_t>(data[offset + 4U])) |
        static_cast<std::uint16_t>(std::to_integer<std::uint8_t>(data[offset + 5U]) << 8U);
    const std::int16_t candidate = std::bit_cast<std::int16_t>(encoded);
    if (candidate < 0 || candidate > 30 || surfaceId.has_value()) {
      return {.error = "invalid CPlugMaterial surface-ID chunk in " + path};
    }
    surfaceId = candidate;
  }
  if (!surfaceId.has_value()) {
    return {.error = "CPlugMaterial surface-ID chunk is unavailable in " + path};
  }
  return {.surfaceId = *surfaceId, .error = {}};
}

struct ExternalPathsResult final {
  std::vector<std::string> paths;
  std::string error;

  explicit operator bool() const noexcept { return error.empty(); }
};

ExternalPathsResult readExternalPaths(const pak::VirtualFileSystem& vfs,
                                      const std::string_view documentPath) {
  pak::VfsOpenResult opened = vfs.openFile(documentPath);
  if (!opened) {
    ExternalPathsResult result;
    result.error = opened.error;
    return result;
  }
  const gbx::DocumentProbeResult probed = gbx::probeDocument(*opened.input, false);
  if (!probed) {
    ExternalPathsResult result;
    result.error = probed.error;
    return result;
  }
  ExternalPathsResult result;
  result.paths.reserve(probed.document.externalNodes.size());
  for (const gbx::ExternalNode& node : probed.document.externalNodes) {
    if (!node.resourceIndex.has_value()) {
      result.paths.push_back(gbx::externalNodePath(probed.document, node));
    }
  }
  return result;
}

int diffuseTextureScore(const std::string_view path) {
  const std::string normalized = lowercase(path);
  constexpr std::string_view suffix = ".texture.gbx";
  if (!endsWith(normalized, suffix)) {
    return -1;
  }
  const std::size_t separator = normalized.find_last_of("\\/");
  std::string_view name = normalized;
  if (separator != std::string::npos) {
    name.remove_prefix(separator + 1U);
  }
  name.remove_suffix(suffix.size());
  if (endsWith(name, "d") || name.find("diff") != std::string_view::npos) {
    return 100;
  }
  if (endsWith(name, "n") || endsWith(name, "s") || endsWith(name, "o") || endsWith(name, "l") ||
      name.find("mask") != std::string_view::npos ||
      name.find("fresnel") != std::string_view::npos ||
      name.find("reflect") != std::string_view::npos ||
      name.find("prelight") != std::string_view::npos) {
    return 5;
  }
  return 20;
}

std::optional<std::string> selectDiffuseTextureDocument(const std::vector<std::string>& paths) {
  int bestScore = -1;
  const std::string* best = nullptr;
  for (const std::string& path : paths) {
    const int score = diffuseTextureScore(path);
    if (score > bestScore) {
      bestScore = score;
      best = &path;
    }
  }
  if (best == nullptr) {
    return std::nullopt;
  }
  return *best;
}

std::optional<std::string> selectImage(const std::vector<std::string>& paths) {
  const std::string* tga = nullptr;
  for (const std::string& path : paths) {
    const std::string normalized = lowercase(path);
    if (endsWith(normalized, ".dds")) {
      return path;
    }
    if (tga == nullptr && endsWith(normalized, ".tga")) {
      tga = &path;
    }
  }
  if (tga == nullptr) {
    return std::nullopt;
  }
  return *tga;
}

std::string resolveImagePath(const std::string_view textureDocumentPath,
                             const std::string_view imageReference) {
  const std::string normalized = pak::normalizeArchivePath(imageReference);
  if (normalized.starts_with("media\\") || normalized.find("\\media\\") != std::string::npos) {
    return std::string(imageReference);
  }
  const std::size_t separator = textureDocumentPath.find_last_of("\\/");
  if (separator == std::string_view::npos) {
    return std::string(imageReference);
  }
  return std::string(textureDocumentPath.substr(0, separator + 1U)) + std::string(imageReference);
}

image::DecodeResult decodeImage(const std::string_view path,
                                const std::span<const std::byte> bytes) {
  const std::string normalized = lowercase(path);
  if (endsWith(normalized, ".dds")) {
    return image::decodeDds(bytes);
  }
  if (endsWith(normalized, ".tga")) {
    return image::decodeTga(bytes);
  }
  image::DecodeResult result;
  result.error = "unsupported material image extension";
  return result;
}

} // namespace

SolidMaterialResolveResult resolveSolidMaterials(const gbx::SolidModel& solid,
                                                 const pak::VirtualFileSystem& vfs,
                                                 const LooseFileReader& looseFileReader) {
  const std::array<const gbx::SolidModel*, 1> solids{&solid};
  SolidMaterialBatchResolveResult batch = resolveSolidMaterialBatch(solids, vfs, looseFileReader);
  if (!batch) {
    return failed(std::move(batch.error));
  }
  SolidMaterialResolveResult result;
  result.materials.textures = std::move(batch.materials.textures);
  result.materials.visualTextureIndices = std::move(batch.materials.visualTextureIndices.front());
  return result;
}

SolidMaterialBatchResolveResult
resolveSolidMaterialBatch(const std::span<const gbx::SolidModel* const> solids,
                          const pak::VirtualFileSystem& vfs,
                          const LooseFileReader& looseFileReader) {
  if (!looseFileReader) {
    return batchFailed("loose GameData reader is unavailable");
  }

  SolidMaterialBatchResolveResult result;
  result.materials.visualTextureIndices.resize(solids.size());
  std::unordered_map<std::string, std::size_t> materialCache;
  std::unordered_map<std::string, std::size_t> imageCache;

  for (std::size_t solidIndex = 0; solidIndex < solids.size(); ++solidIndex) {
    const gbx::SolidModel* const solid = solids[solidIndex];
    if (solid == nullptr) {
      return batchFailed("material batch contains a null SolidModel");
    }
    std::vector<std::size_t>& bindings = result.materials.visualTextureIndices[solidIndex];
    bindings.assign(solid->visuals.size(), kNoVisualTexture);
    for (std::size_t visualIndex = 0; visualIndex < solid->visuals.size(); ++visualIndex) {
      const std::string& materialReference = solid->visuals[visualIndex].materialReference;
      if (materialReference.empty()) {
        continue;
      }
      const std::string cacheKey = pak::normalizeArchivePath(materialReference);
      if (const auto existing = materialCache.find(cacheKey); existing != materialCache.end()) {
        bindings[visualIndex] = existing->second;
        continue;
      }

      const std::string materialPath = materialDocumentPath(materialReference);
      const ExternalPathsResult materialPaths = readExternalPaths(vfs, materialPath);
      if (!materialPaths) {
        return batchFailed("unable to read material " + materialReference + ": " +
                           materialPaths.error);
      }
      const std::optional<std::string> textureDocument =
          selectDiffuseTextureDocument(materialPaths.paths);
      if (!textureDocument.has_value()) {
        return batchFailed("material has no diffuse texture reference: " + materialReference);
      }

      const ExternalPathsResult texturePaths = readExternalPaths(vfs, *textureDocument);
      if (!texturePaths) {
        return batchFailed("unable to read texture document " + *textureDocument + ": " +
                           texturePaths.error);
      }
      const std::optional<std::string> imageReference = selectImage(texturePaths.paths);
      if (!imageReference.has_value()) {
        const bool isVideo =
            std::any_of(texturePaths.paths.begin(), texturePaths.paths.end(),
                        [](const std::string& path) { return endsWith(lowercase(path), ".bik"); });
        if (isVideo) {
          materialCache.emplace(cacheKey, kNoVisualTexture);
          continue;
        }
        return batchFailed("texture document has no DDS/TGA reference: " + *textureDocument);
      }
      const std::string imagePath = resolveImagePath(*textureDocument, *imageReference);
      const std::string imageKey = pak::normalizeArchivePath(imagePath);
      if (const auto existing = imageCache.find(imageKey); existing != imageCache.end()) {
        bindings[visualIndex] = existing->second;
        materialCache.emplace(cacheKey, existing->second);
        continue;
      }
      std::optional<std::vector<std::byte>> imageBytes = looseFileReader(imagePath);
      if (!imageBytes.has_value()) {
        pak::VfsReadResult packed = vfs.readFile(imagePath);
        if (packed) {
          imageBytes = std::move(packed.data);
        }
      }
      if (!imageBytes.has_value()) {
        return batchFailed("material image is unavailable: " + imagePath);
      }
      image::DecodeResult decoded = decodeImage(imagePath, *imageBytes);
      if (!decoded) {
        return batchFailed("unable to decode " + imagePath + ": " + decoded.error);
      }

      const std::size_t textureIndex = result.materials.textures.size();
      ResolvedMaterialTexture texture;
      texture.materialReference = materialReference;
      texture.textureDocumentPath = *textureDocument;
      texture.imagePath = imagePath;
      texture.image = std::move(decoded.image);
      result.materials.textures.push_back(std::move(texture));
      bindings[visualIndex] = textureIndex;
      materialCache.emplace(cacheKey, textureIndex);
      imageCache.emplace(imageKey, textureIndex);
    }
  }
  return result;
}

SolidSurfaceMaterialResolveResult
resolveSolidSurfaceMaterials(const std::span<gbx::SolidModel* const> solids,
                             const pak::VirtualFileSystem& vfs) {
  std::unordered_map<std::string, std::int16_t> cache;
  for (gbx::SolidModel* const solid : solids) {
    if (solid == nullptr) {
      return surfaceFailed("surface-material batch contains a null SolidModel");
    }
    if (solid->surfaceMaterials.size() != solid->surfaceMaterialReferences.size()) {
      return surfaceFailed("CPlugSurface material IDs and references are inconsistent");
    }
    for (std::size_t index = 0; index < solid->surfaceMaterials.size(); ++index) {
      const std::string& reference = solid->surfaceMaterialReferences[index];
      if (reference.empty()) {
        continue;
      }
      const std::string cacheKey = pak::normalizeArchivePath(reference);
      if (const auto existing = cache.find(cacheKey); existing != cache.end()) {
        solid->surfaceMaterials[index] = existing->second;
        continue;
      }
      const MaterialSurfaceIdResult resolved = readMaterialSurfaceId(vfs, reference);
      if (!resolved) {
        return surfaceFailed(resolved.error);
      }
      solid->surfaceMaterials[index] = resolved.surfaceId;
      cache.emplace(cacheKey, resolved.surfaceId);
    }
  }
  return {};
}

} // namespace tmnf::content
