#pragma once

#include "tmnf/gbx/header.hpp"
#include "tmnf/gbx/input.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace tmnf::gbx {

struct ReferenceDirectory final {
  std::int32_t parentIndex = -1;
  std::string name;
};

struct ExternalNode final {
  std::int32_t flags = 0;
  std::string name;
  std::optional<std::int32_t> resourceIndex;
  std::int32_t nodeIndex = 0;
  bool useFile = false;
  std::int32_t directoryIndex = -1;
};

struct DocumentProbe final {
  Header header;
  std::int32_t numNodes = 0;
  std::int32_t ancestorLevel = 0;
  std::vector<ReferenceDirectory> directories;
  std::vector<ExternalNode> externalNodes;
  std::size_t bodyHeaderOffset = 0;
  std::size_t bodyDataOffset = 0;
  std::optional<std::uint32_t> bodyUncompressedSize;
  std::optional<std::uint32_t> bodyCompressedSize;
  std::optional<std::uint32_t> dummyWriteSeed;
  std::optional<std::uint32_t> firstChunkId;
  bool fullyConsumed = false;
};

struct DocumentProbeResult final {
  DocumentProbe document;
  std::string error;

  [[nodiscard]] explicit operator bool() const noexcept { return error.empty(); }
};

// Reconstructs the path stored by a reference-table node from its directory chain.
[[nodiscard]] std::string externalNodePath(const DocumentProbe& document, const ExternalNode& node);

// Parses the common GBX header and reference table. For an uncompressed body,
// this also applies the root-node dummy-write seed before reading its first
// chunk. When consumeRemaining is true, the rest of the logical file is read
// to validate the outer NadeoPak zlib stream; nested node seeds are not yet
// synthesized, so complex files can intentionally fail that final phase.
[[nodiscard]] DocumentProbeResult probeDocument(Input& input, bool consumeRemaining = false);

} // namespace tmnf::gbx
