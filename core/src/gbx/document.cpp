#include "tmnf/gbx/document.hpp"

#include "tmnf/gbx/class_registry.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <limits>
#include <span>
#include <utility>

namespace tmnf::gbx {
namespace {

constexpr std::uint32_t kMaximumUserDataSize = 1U << 24U;
constexpr std::int32_t kMaximumCollectionCount = 1 << 20;
constexpr std::size_t kMaximumDirectoryDepth = 256;

class Reader final {
public:
  explicit Reader(Input& input) : input_(input) {}

  bool readBytes(const std::span<std::byte> destination) {
    if (destination.size() > input_.size() - input_.position()) {
      return fail("GBX read exceeds the declared file size");
    }
    if (!input_.readBytes(destination)) {
      const std::string_view detail = input_.error();
      return fail(detail.empty() ? "unable to read GBX input" : std::string(detail));
    }
    return true;
  }

  bool readU8(std::uint8_t& value) {
    std::byte byte{};
    if (!readBytes(std::span(&byte, 1))) {
      return false;
    }
    value = std::to_integer<std::uint8_t>(byte);
    return true;
  }

  bool readU32(std::uint32_t& value) {
    std::array<std::byte, 4> bytes{};
    if (!readBytes(bytes)) {
      return false;
    }
    value = readLeU32(bytes.data());
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

  bool readString(std::string& value) {
    std::int32_t length = 0;
    if (!readI32(length) || length < 0 || length > kMaximumCollectionCount) {
      return fail("invalid GBX string length");
    }
    value.resize(static_cast<std::size_t>(length));
    return readBytes(std::span(reinterpret_cast<std::byte*>(value.data()), value.size()));
  }

  bool skip(std::size_t count) {
    std::array<std::byte, 4096> buffer{};
    while (count != 0) {
      const std::size_t chunk = std::min(count, buffer.size());
      if (!readBytes(std::span(buffer).first(chunk))) {
        return false;
      }
      count -= chunk;
    }
    return true;
  }

  bool fail(std::string message) {
    if (error_.empty()) {
      error_ = std::move(message);
    }
    return false;
  }

  [[nodiscard]] std::string takeError() { return std::move(error_); }

private:
  static std::uint32_t readLeU32(const std::byte* const bytes) noexcept {
    std::uint32_t value = 0;
    for (std::size_t index = 0; index < 4; ++index) {
      value |= static_cast<std::uint32_t>(std::to_integer<unsigned char>(bytes[index]))
               << (index * 8U);
    }
    return value;
  }

  Input& input_;
  std::string error_;
};

std::uint32_t readLeU32(const std::byte* const bytes) noexcept {
  std::uint32_t value = 0;
  for (std::size_t index = 0; index < 4; ++index) {
    value |= static_cast<std::uint32_t>(std::to_integer<unsigned char>(bytes[index]))
             << (index * 8U);
  }
  return value;
}

std::array<std::byte, 4> writeLeU32(const std::uint32_t value) noexcept {
  std::array<std::byte, 4> bytes{};
  for (std::size_t index = 0; index < bytes.size(); ++index) {
    bytes[index] = static_cast<std::byte>((value >> (index * 8U)) & 0xffU);
  }
  return bytes;
}

bool readDirectories(Reader& reader, std::vector<ReferenceDirectory>& directories,
                     const std::int32_t parentIndex, const std::size_t depth) {
  if (depth > kMaximumDirectoryDepth) {
    return reader.fail("GBX reference-table directory nesting is too deep");
  }

  std::int32_t childCount = 0;
  if (!reader.readI32(childCount) || childCount < 0 ||
      childCount > kMaximumCollectionCount - static_cast<std::int32_t>(directories.size())) {
    return reader.fail("invalid GBX reference-table directory count");
  }
  for (std::int32_t index = 0; index < childCount; ++index) {
    ReferenceDirectory directory;
    directory.parentIndex = parentIndex;
    if (!reader.readString(directory.name)) {
      return false;
    }
    const std::int32_t directoryIndex = static_cast<std::int32_t>(directories.size());
    directories.push_back(std::move(directory));
    if (!readDirectories(reader, directories, directoryIndex, depth + 1)) {
      return false;
    }
  }
  return true;
}

DocumentProbeResult failed(Reader& reader, const char* const fallback) {
  DocumentProbeResult result;
  result.error = reader.takeError();
  if (result.error.empty()) {
    result.error = fallback;
  }
  return result;
}

} // namespace

std::string externalNodePath(const DocumentProbe& document, const ExternalNode& node) {
  std::vector<std::string_view> components;
  std::int32_t directoryIndex = node.directoryIndex;
  while (directoryIndex >= 0 &&
         directoryIndex < static_cast<std::int32_t>(document.directories.size()) &&
         components.size() <= document.directories.size()) {
    const ReferenceDirectory& directory =
        document.directories[static_cast<std::size_t>(directoryIndex)];
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

DocumentProbeResult probeDocument(Input& input, const bool consumeRemaining) {
  Reader reader(input);
  constexpr std::size_t kVersion6PrefixSize = 17;
  std::array<std::byte, kVersion6PrefixSize> prefix{};
  if (!reader.readBytes(prefix)) {
    return failed(reader, "truncated GBX header");
  }
  if (std::to_integer<char>(prefix[0]) != 'G' || std::to_integer<char>(prefix[1]) != 'B' ||
      std::to_integer<char>(prefix[2]) != 'X') {
    reader.fail("missing GBX magic");
    return failed(reader, "missing GBX magic");
  }

  const std::uint32_t userDataSize = readLeU32(prefix.data() + 13);
  if (userDataSize > kMaximumUserDataSize || userDataSize > input.size() - kVersion6PrefixSize) {
    reader.fail("invalid GBX user-data size");
    return failed(reader, "invalid GBX user-data size");
  }
  std::vector<std::byte> headerBytes(kVersion6PrefixSize + userDataSize);
  std::copy(prefix.begin(), prefix.end(), headerBytes.begin());
  if (!reader.readBytes(std::span(headerBytes).subspan(prefix.size()))) {
    return failed(reader, "truncated GBX user data");
  }

  const ParseResult parsedHeader = parseHeader(headerBytes);
  if (!parsedHeader) {
    DocumentProbeResult result;
    result.error = parsedHeader.error;
    return result;
  }

  DocumentProbeResult result;
  DocumentProbe& document = result.document;
  document.header = parsedHeader.header;
  if (!reader.readI32(document.numNodes) || document.numNodes < 0 ||
      document.numNodes > kMaximumCollectionCount) {
    reader.fail("invalid GBX node count");
    return failed(reader, "invalid GBX node count");
  }

  std::int32_t externalNodeCount = 0;
  if (!reader.readI32(externalNodeCount) || externalNodeCount < 0 ||
      externalNodeCount > kMaximumCollectionCount) {
    reader.fail("invalid GBX external-node count");
    return failed(reader, "invalid GBX external-node count");
  }
  if (externalNodeCount != 0) {
    if (!reader.readI32(document.ancestorLevel) ||
        !readDirectories(reader, document.directories, -1, 0)) {
      return failed(reader, "invalid GBX reference table");
    }

    document.externalNodes.reserve(static_cast<std::size_t>(externalNodeCount));
    for (std::int32_t index = 0; index < externalNodeCount; ++index) {
      ExternalNode node;
      if (!reader.readI32(node.flags)) {
        return failed(reader, "truncated GBX external-node table");
      }
      if ((node.flags & 4) != 0) {
        std::int32_t resourceIndex = 0;
        if (!reader.readI32(resourceIndex)) {
          return failed(reader, "truncated GBX resource reference");
        }
        node.resourceIndex = resourceIndex;
      } else if (!reader.readString(node.name)) {
        return failed(reader, "truncated GBX file reference");
      }
      std::uint32_t useFile = 0;
      if (!reader.readI32(node.nodeIndex) || !reader.readU32(useFile) || useFile > 1) {
        return failed(reader, "truncated GBX external-node metadata");
      }
      node.useFile = useFile != 0;
      if (!node.resourceIndex.has_value()) {
        std::int32_t encodedDirectory = 0;
        if (!reader.readI32(encodedDirectory) || encodedDirectory < 0 ||
            encodedDirectory > static_cast<std::int32_t>(document.directories.size())) {
          reader.fail("invalid GBX external-node directory index");
          return failed(reader, "invalid GBX external-node directory index");
        }
        node.directoryIndex = encodedDirectory - 1;
      }
      document.externalNodes.push_back(std::move(node));
    }
  }

  document.bodyHeaderOffset = input.position();
  if (document.header.bodyCompression == 'C') {
    std::uint32_t uncompressedSize = 0;
    std::uint32_t compressedSize = 0;
    if (!reader.readU32(uncompressedSize) || !reader.readU32(compressedSize) ||
        compressedSize > input.size() - input.position()) {
      reader.fail("invalid compressed GBX body sizes");
      return failed(reader, "invalid compressed GBX body sizes");
    }
    document.bodyUncompressedSize = uncompressedSize;
    document.bodyCompressedSize = compressedSize;
    document.bodyDataOffset = input.position();
  } else if (document.header.bodyCompression == 'U') {
    document.bodyDataOffset = input.position();
    document.dummyWriteSeed = dummyWriteSeedClassId(document.header.classId);
    if (document.dummyWriteSeed.has_value()) {
      const std::array seed = writeLeU32(*document.dummyWriteSeed);
      input.initialize(seed);
    }
    std::uint32_t firstChunkId = 0;
    if (!reader.readU32(firstChunkId)) {
      return failed(reader, "missing GBX body chunk");
    }
    document.firstChunkId = firstChunkId;
  } else {
    reader.fail("unsupported GBX body-compression marker");
    return failed(reader, "unsupported GBX body-compression marker");
  }

  if (consumeRemaining) {
    if (!reader.skip(input.size() - input.position())) {
      return failed(reader, "unable to consume GBX body");
    }
    document.fullyConsumed = true;
  }
  return result;
}

} // namespace tmnf::gbx
