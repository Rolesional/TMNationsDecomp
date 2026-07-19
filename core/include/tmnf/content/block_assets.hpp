#pragma once

#include "tmnf/gbx/block_info.hpp"
#include "tmnf/pak/vfs.hpp"

#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace tmnf::content {

enum class BlockSolidRole {
  Any,
  Ground,
  Air,
};

struct BlockSolidAsset final {
  std::string logicalPath;
  BlockSolidRole role = BlockSolidRole::Any;
};

struct BlockAssetGraph final {
  std::string blockName;
  std::string blockInfoPath;
  std::optional<gbx::BlockInfoMetadata> metadata;
  std::array<std::int32_t, 2> groundFootprint{1, 1};
  std::array<std::int32_t, 2> airFootprint{1, 1};
  std::vector<BlockSolidAsset> solids;
  std::vector<BlockSolidAsset> triggerSolids;
  std::vector<std::string> unresolvedReferences;
  std::size_t probedNodeCount = 0;
};

struct BlockAssetResolveResult final {
  BlockAssetGraph graph;
  std::string error;

  [[nodiscard]] explicit operator bool() const noexcept { return error.empty(); }
};

// Resolves a reference-table path against the directory containing the current
// GBX, after applying the table's ancestor level.
[[nodiscard]] std::optional<std::string> resolveReferencePath(std::string_view currentLogicalPath,
                                                              std::int32_t ancestorLevel,
                                                              std::string_view referencedPath);

// Finds the TMF construction BlockInfo for a named block and follows only the
// BlockInfo/Mobil branches of its GBX reference graph to visible and trigger
// CPlugSolid files, keeping trigger geometry separate from rendered geometry.
[[nodiscard]] BlockAssetResolveResult resolveBlockAssets(std::string_view collection,
                                                         std::string_view blockName,
                                                         const pak::VirtualFileSystem& vfs);

} // namespace tmnf::content
