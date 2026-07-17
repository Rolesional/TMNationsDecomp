#pragma once

#include "tmnf/gbx/solid.hpp"
#include "tmnf/image/image.hpp"
#include "tmnf/pak/vfs.hpp"

#include <cstddef>
#include <functional>
#include <limits>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace tmnf::content {

inline constexpr std::size_t kNoVisualTexture = std::numeric_limits<std::size_t>::max();

using LooseFileReader =
    std::function<std::optional<std::vector<std::byte>>(std::string_view virtualPath)>;

struct ResolvedMaterialTexture final {
  std::string materialReference;
  std::string textureDocumentPath;
  std::string imagePath;
  image::RgbaImage image;
};

struct SolidMaterialSet final {
  std::vector<ResolvedMaterialTexture> textures;
  // One entry per SolidModel::visuals element. Repeated LOD materials share a texture.
  std::vector<std::size_t> visualTextureIndices;
};

struct SolidMaterialResolveResult final {
  SolidMaterialSet materials;
  std::string error;

  [[nodiscard]] explicit operator bool() const noexcept { return error.empty(); }
};

struct SolidMaterialBatch final {
  std::vector<ResolvedMaterialTexture> textures;
  // One visual-texture binding vector for each input SolidModel pointer.
  std::vector<std::vector<std::size_t>> visualTextureIndices;
};

struct SolidMaterialBatchResolveResult final {
  SolidMaterialBatch materials;
  std::string error;

  [[nodiscard]] explicit operator bool() const noexcept { return error.empty(); }
};

// Follows CPlugMaterial -> CPlugBitmap reference tables in the mounted archives, then
// asks looseFileReader/VFS for the referenced DDS/TGA. Bink-backed animated signs remain
// unbound until video decoding exists. The diffuse/base texture is selected from the
// material's ordered external texture nodes.
[[nodiscard]] SolidMaterialResolveResult
resolveSolidMaterials(const gbx::SolidModel& solid, const pak::VirtualFileSystem& vfs,
                      const LooseFileReader& looseFileReader);

// Resolves a group of solids into one shared texture table. Material and image
// paths are cached across all models so repeated Stadium textures decode once.
[[nodiscard]] SolidMaterialBatchResolveResult
resolveSolidMaterialBatch(std::span<const gbx::SolidModel* const> solids,
                          const pak::VirtualFileSystem& vfs,
                          const LooseFileReader& looseFileReader);

} // namespace tmnf::content
