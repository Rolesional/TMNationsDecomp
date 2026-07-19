#include "tmnf/content/block_assets.hpp"

#include "tmnf/gbx/class_registry.hpp"
#include "tmnf/gbx/document.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <optional>
#include <string_view>
#include <unordered_set>
#include <utility>

namespace tmnf::content {
namespace {

constexpr std::uint32_t kCGameCtnBlockInfo = 0x0304E000U;
constexpr std::uint32_t kCGameCtnBlockInfoMobil = 0x03122000U;
constexpr std::uint32_t kCPlugSolid = 0x09005000U;
constexpr std::uint32_t kCSceneMobil = 0x0A011000U;
constexpr std::size_t kMaximumReferenceDepth = 16;

struct BlockInfoType final {
  std::string_view directorySuffix;
  std::string_view extensionSuffix;
};

constexpr std::array kBlockInfoTypes{
    BlockInfoType{"Classic", "Classic"},   BlockInfoType{"Road", "Road"},
    BlockInfoType{"Clip", "Clip"},         BlockInfoType{"Flat", "Flat"},
    BlockInfoType{"Frontier", "Frontier"}, BlockInfoType{"Pylon", "Pylon"},
    BlockInfoType{"RectAsym", "RectAsym"}, BlockInfoType{"Slope", "Slope"},
};

BlockAssetResolveResult failed(std::string message) {
  BlockAssetResolveResult result;
  result.error = std::move(message);
  return result;
}

std::string lowercase(std::string_view value) {
  std::string result(value);
  std::ranges::transform(result, result.begin(), [](const unsigned char character) {
    if (character == '/') {
      return '\\';
    }
    return static_cast<char>(std::tolower(character));
  });
  return result;
}

std::vector<std::string> splitPath(const std::string_view path) {
  std::vector<std::string> components;
  std::size_t begin = 0;
  while (begin < path.size()) {
    const std::size_t separator = path.find_first_of("\\/", begin);
    const std::size_t end = separator == std::string_view::npos ? path.size() : separator;
    if (end != begin) {
      components.emplace_back(path.substr(begin, end - begin));
    }
    if (separator == std::string_view::npos) {
      break;
    }
    begin = separator + 1U;
  }
  return components;
}

std::string joinPath(const std::vector<std::string>& components) {
  std::string result;
  for (const std::string& component : components) {
    if (!result.empty()) {
      result.push_back('\\');
    }
    result += component;
  }
  return result;
}

bool isClassOrDerived(std::uint32_t classId, const std::uint32_t baseClassId) {
  for (std::size_t depth = 0; depth < 32; ++depth) {
    if (classId == baseClassId) {
      return true;
    }
    const std::optional<std::uint32_t> parent = gbx::classParentId(classId);
    if (!parent.has_value() || *parent == classId) {
      return false;
    }
    classId = *parent;
  }
  return false;
}

bool ignoredVisualReference(const std::string_view path) {
  const std::string normalized = lowercase(path);
  return normalized.find("editorhelper") != std::string::npos ||
         normalized.find("helperarrow") != std::string::npos ||
         normalized.find("\\clip.solid.gbx") != std::string::npos ||
         normalized.find("\\clipground.solid.gbx") != std::string::npos ||
         normalized.find("\\clipair.solid.gbx") != std::string::npos ||
         normalized.find("lightsystem") != std::string::npos ||
         normalized.find("soundsystem") != std::string::npos;
}

bool isTriggerSolid(const std::string_view path) {
  const std::string normalized = lowercase(path);
  const std::size_t separator = normalized.find_last_of('\\');
  const std::string_view name = separator == std::string::npos
                                    ? std::string_view(normalized)
                                    : std::string_view(normalized).substr(separator + 1U);
  return name.find("trigger") != std::string_view::npos && name.ends_with(".solid.gbx");
}

BlockSolidRole solidRole(const std::string_view path) {
  const std::string normalized = lowercase(path);
  const std::size_t separator = normalized.find_last_of('\\');
  const std::string_view fileName = separator == std::string::npos
                                        ? std::string_view(normalized)
                                        : std::string_view(normalized).substr(separator + 1U);
  if (fileName.find("ground") != std::string_view::npos) {
    return BlockSolidRole::Ground;
  }
  if (fileName.find("air") != std::string_view::npos) {
    return BlockSolidRole::Air;
  }
  return BlockSolidRole::Any;
}

std::vector<std::string> blockNameAliases(const std::string_view blockName) {
  std::vector<std::string> aliases{std::string(blockName)};
  if (blockName == "StadiumFabricPillarCornerOut") {
    aliases.emplace_back("StadiumFabricStraight1x1Ramp");
  }
  constexpr std::string_view kStartLine = "StartLine";
  constexpr std::string_view kFinishLine = "FinishLine";
  constexpr std::string_view kStartFinishLine = "StartFinishLine";
  if (blockName.ends_with(kStartFinishLine)) {
    aliases.emplace_back(
        std::string(blockName.substr(0, blockName.size() - kStartFinishLine.size())) + "MultiLap");
  }
  if (blockName.ends_with(kStartLine)) {
    aliases.emplace_back(std::string(blockName.substr(0, blockName.size() - kStartLine.size())) +
                         "Start");
  }
  if (blockName.ends_with(kFinishLine)) {
    aliases.emplace_back(std::string(blockName.substr(0, blockName.size() - kFinishLine.size())) +
                         "Finish");
  }
  return aliases;
}

std::optional<std::string> findBlockInfoPath(const std::string_view collection,
                                             const std::string_view blockName,
                                             const pak::VirtualFileSystem& vfs) {
  for (const std::string& alias : blockNameAliases(blockName)) {
    for (const BlockInfoType type : kBlockInfoTypes) {
      std::string path(collection);
      path += "\\ConstructionBlockInfo\\ConstructionBlockInfo";
      path += type.directorySuffix;
      path.push_back('\\');
      path += alias;
      path += ".TMED";
      path += type.extensionSuffix;
      path += ".Gbx";
      if (vfs.contains(path)) {
        return path;
      }
    }
  }
  return std::nullopt;
}

class GraphResolver final {
public:
  GraphResolver(const pak::VirtualFileSystem& vfs, BlockAssetGraph& graph)
      : vfs_(vfs), graph_(graph) {}

  bool visit(const std::string& path, const std::size_t depth, const bool root = false) {
    if (depth > kMaximumReferenceDepth) {
      graph_.unresolvedReferences.push_back(path + " (reference depth exceeded)");
      return true;
    }
    const std::string normalized = pak::normalizeArchivePath(path);
    if (!visited_.insert(normalized).second) {
      return true;
    }
    pak::VfsOpenResult opened = vfs_.openFile(path);
    if (!opened) {
      graph_.unresolvedReferences.push_back(path);
      return root ? false : true;
    }
    gbx::DocumentProbeResult probed = gbx::probeDocument(*opened.input, false);
    if (!probed) {
      graph_.unresolvedReferences.push_back(path + " (" + probed.error + ")");
      return root ? false : true;
    }
    ++graph_.probedNodeCount;
    const std::uint32_t classId = probed.document.header.classId;
    if (isClassOrDerived(classId, kCPlugSolid)) {
      if (!ignoredVisualReference(path) && solidPaths_.insert(normalized).second) {
        auto& solids = isTriggerSolid(path) ? graph_.triggerSolids : graph_.solids;
        solids.push_back({path, solidRole(path)});
      }
      return true;
    }
    const bool traversable = isClassOrDerived(classId, kCGameCtnBlockInfo) ||
                             isClassOrDerived(classId, kCSceneMobil) ||
                             classId == kCGameCtnBlockInfoMobil;
    if (!traversable) {
      return true;
    }

    for (const gbx::ExternalNode& node : probed.document.externalNodes) {
      if (node.resourceIndex.has_value()) {
        continue;
      }
      const std::string referenced = gbx::externalNodePath(probed.document, node);
      const std::optional<std::string> resolved =
          resolveReferencePath(path, probed.document.ancestorLevel, referenced);
      if (!resolved.has_value()) {
        graph_.unresolvedReferences.push_back(path + " -> " + referenced);
        continue;
      }
      if (ignoredVisualReference(*resolved)) {
        continue;
      }
      visit(*resolved, depth + 1U);
    }
    return true;
  }

private:
  const pak::VirtualFileSystem& vfs_;
  BlockAssetGraph& graph_;
  std::unordered_set<std::string> visited_;
  std::unordered_set<std::string> solidPaths_;
};

} // namespace

std::optional<std::string> resolveReferencePath(const std::string_view currentLogicalPath,
                                                const std::int32_t ancestorLevel,
                                                const std::string_view referencedPath) {
  if (ancestorLevel < 0 || referencedPath.empty()) {
    return std::nullopt;
  }
  std::vector<std::string> components = splitPath(currentLogicalPath);
  if (components.empty()) {
    return std::nullopt;
  }
  components.pop_back();
  if (static_cast<std::size_t>(ancestorLevel) > components.size()) {
    return std::nullopt;
  }
  components.resize(components.size() - static_cast<std::size_t>(ancestorLevel));
  for (std::string& component : splitPath(referencedPath)) {
    if (component == ".") {
      continue;
    }
    if (component == "..") {
      if (components.empty()) {
        return std::nullopt;
      }
      components.pop_back();
      continue;
    }
    components.push_back(std::move(component));
  }
  if (components.empty()) {
    return std::nullopt;
  }
  return joinPath(components);
}

BlockAssetResolveResult resolveBlockAssets(const std::string_view collection,
                                           const std::string_view blockName,
                                           const pak::VirtualFileSystem& vfs) {
  if (collection.empty() || blockName.empty()) {
    return failed("block collection and name are required");
  }
  const std::optional<std::string> blockInfoPath = findBlockInfoPath(collection, blockName, vfs);
  if (!blockInfoPath.has_value()) {
    return failed("construction BlockInfo not found for " + std::string(blockName));
  }
  BlockAssetResolveResult result;
  result.graph.blockName = blockName;
  result.graph.blockInfoPath = *blockInfoPath;
  pak::VfsOpenResult metadataInput = vfs.openFile(*blockInfoPath);
  if (!metadataInput) {
    return failed("unable to open construction BlockInfo metadata for " + std::string(blockName));
  }
  gbx::BlockInfoParseResult metadata = gbx::parseBlockInfo(*metadataInput.input);
  if (metadata) {
    result.graph.groundFootprint = metadata.blockInfo.groundFootprint;
    result.graph.airFootprint = metadata.blockInfo.airFootprint;
    if (metadata.blockInfo.waypointType != gbx::BlockWaypointType::None) {
      result.graph.metadata = std::move(metadata.blockInfo);
    }
  }
  GraphResolver resolver(vfs, result.graph);
  if (!resolver.visit(*blockInfoPath, 0, true)) {
    return failed("unable to probe construction BlockInfo for " + std::string(blockName));
  }
  return result;
}

} // namespace tmnf::content
