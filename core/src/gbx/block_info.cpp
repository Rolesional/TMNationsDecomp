#include "tmnf/gbx/block_info.hpp"

#include "tmnf/gbx/class_registry.hpp"
#include "tmnf/gbx/document.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <utility>

namespace tmnf::gbx {
namespace {

constexpr std::uint32_t kFacade = 0xFACADE01U;
constexpr std::uint32_t kSkip = 0x534B4950U;
constexpr std::uint32_t kBlockInfoClassId = 0x0304E000U;
constexpr std::uint32_t kBlockUnitInfoClassId = 0x03036000U;
constexpr std::uint32_t kSceneMobilClassId = 0x0A011000U;
constexpr std::uint32_t kPlugSolidClassId = 0x09005000U;
constexpr std::int32_t kMaximumCount = 1 << 20;

class Reader final {
public:
  explicit Reader(Input& input) : input_(input) {}

  bool readBytes(const std::span<std::byte> destination) {
    if (input_.position() > input_.size() ||
        destination.size() > input_.size() - input_.position()) {
      return fail("GBX BlockInfo read exceeds the declared file size");
    }
    if (!input_.readBytes(destination)) {
      const std::string_view detail = input_.error();
      return fail(detail.empty() ? "unable to read GBX BlockInfo" : std::string(detail));
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
    value = static_cast<std::uint16_t>(std::to_integer<std::uint8_t>(bytes[0])) |
            static_cast<std::uint16_t>(std::to_integer<std::uint8_t>(bytes[1]) << 8U);
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
      value |= static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(bytes[index]))
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
      return fail("invalid GBX BlockInfo boolean");
    }
    value = encoded != 0U;
    return true;
  }

  bool readString(std::string& value) {
    std::int32_t length = 0;
    if (!readCount(length, "string")) {
      return false;
    }
    value.resize(static_cast<std::size_t>(length));
    return readBytes(std::span(reinterpret_cast<std::byte*>(value.data()), value.size()));
  }

  bool readCount(std::int32_t& count, const char* description) {
    if (!readI32(count) || count < 0 || count > kMaximumCount) {
      return fail(std::string("invalid GBX BlockInfo ") + description + " count");
    }
    return true;
  }

  bool skip(std::size_t count) {
    std::array<std::byte, 1024> buffer{};
    while (count != 0U) {
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
  Input& input_;
  std::string error_;
};

bool isClassOrDerived(std::uint32_t classId, const std::uint32_t baseClassId) {
  for (std::size_t depth = 0; depth <= classRegistry().size(); ++depth) {
    if (classId == baseClassId) {
      return true;
    }
    const std::optional<std::uint32_t> parent = classParentId(classId);
    if (!parent.has_value() || *parent == classId) {
      return false;
    }
    classId = *parent;
  }
  return false;
}

class BlockInfoParser final {
public:
  BlockInfoParser(Input& input, const DocumentProbe& document)
      : input_(input), reader_(input), document_(document) {
    records_.emplace(0, document.header.classId);
    for (const ExternalNode& node : document.externalNodes) {
      records_.insert_or_assign(node.nodeIndex, 0U);
    }
  }

  BlockInfoParseResult run() {
    if (!isClassOrDerived(document_.header.classId, kBlockInfoClassId) ||
        !document_.firstChunkId.has_value()) {
      return failed("GBX document is not an uncompressed CGameCtnBlockInfo");
    }
    std::optional<std::uint32_t> firstChunk = document_.firstChunkId;
    while (true) {
      std::uint32_t chunkId = 0;
      if (firstChunk.has_value()) {
        chunkId = *firstChunk;
        firstChunk.reset();
      } else if (!reader_.readU32(chunkId)) {
        return failed(reader_.takeError());
      }
      if (chunkId == kFacade) {
        return failed("CGameCtnBlockInfo ended before waypoint metadata");
      }
      if (!readRootChunk(chunkId)) {
        return failed(reader_.takeError());
      }
      if (complete_) {
        BlockInfoParseResult result;
        result.blockInfo = blockInfo_;
        return result;
      }
    }
  }

private:
  BlockInfoParseResult failed(std::string message) {
    BlockInfoParseResult result;
    result.error = std::move(message);
    if (result.error.empty()) {
      result.error = "unable to parse CGameCtnBlockInfo";
    }
    return result;
  }

  bool initializeNode(const std::uint32_t classId) {
    const std::optional<std::uint32_t> seed = dummyWriteSeedClassId(classId);
    if (!seed.has_value()) {
      return reader_.fail("BlockInfo inline node has no recovered parent-class seed");
    }
    std::array<std::byte, 4> bytes{};
    for (std::size_t index = 0; index < bytes.size(); ++index) {
      bytes[index] = static_cast<std::byte>((*seed >> (index * 8U)) & 0xffU);
    }
    input_.initialize(bytes);
    return true;
  }

  bool readId(std::string& value) {
    if (!idVersion_.has_value()) {
      std::int32_t version = 0;
      if (!reader_.readI32(version) || version != 3) {
        return reader_.fail("unsupported GBX ID version in CGameCtnBlockInfo");
      }
      idVersion_ = version;
    }
    std::uint32_t encoded = 0;
    if (!reader_.readU32(encoded)) {
      return false;
    }
    if (encoded == std::numeric_limits<std::uint32_t>::max()) {
      value.clear();
      return true;
    }
    const std::uint32_t tag = encoded & 0xC0000000U;
    if (tag != 0x40000000U && tag != 0x80000000U) {
      value = std::to_string(encoded);
      return true;
    }
    if ((encoded & 0x3FFFFFFFU) != 0U) {
      const auto existing = ids_.find(encoded);
      if (existing == ids_.end()) {
        return reader_.fail("CGameCtnBlockInfo references an unknown GBX ID");
      }
      value = existing->second;
      return true;
    }
    if (!reader_.readString(value)) {
      return false;
    }
    const std::uint32_t key = encoded + static_cast<std::uint32_t>(ids_.size()) + 1U;
    return ids_.emplace(key, value).second || reader_.fail("duplicate BlockInfo GBX ID");
  }

  bool readNodeReference() {
    std::int32_t index = -1;
    if (!reader_.readI32(index)) {
      return false;
    }
    if (index == -1 || records_.contains(index)) {
      return true;
    }
    if (index < 0) {
      return reader_.fail("invalid CGameCtnBlockInfo node index");
    }
    std::uint32_t classId = 0;
    if (!reader_.readU32(classId)) {
      return false;
    }
    records_.emplace(index, classId);
    if (classId == kBlockUnitInfoClassId) {
      return initializeNode(classId) && readBlockUnitInfo();
    }
    if (classId == kSceneMobilClassId) {
      return initializeNode(classId) && readSceneMobil();
    }
    if (classId == kPlugSolidClassId) {
      return initializeNode(classId) && readHelperSolid();
    }
    return reader_.fail("unsupported inline BlockInfo node class");
  }

  bool readNodeReferenceArray() {
    std::int32_t count = 0;
    if (!reader_.readCount(count, "node-reference")) {
      return false;
    }
    for (std::int32_t index = 0; index < count; ++index) {
      if (!readNodeReference()) {
        return false;
      }
    }
    return true;
  }

  bool skipSkippableChunk(const std::uint32_t chunkId) {
    std::uint32_t marker = 0;
    std::int32_t size = 0;
    if (!reader_.readU32(marker) || marker != kSkip ||
        !reader_.readCount(size, "skippable chunk")) {
      return reader_.fail("unknown unskippable BlockInfo chunk " + std::to_string(chunkId));
    }
    return reader_.skip(static_cast<std::size_t>(size));
  }

  bool readBlockUnitInfo() {
    while (true) {
      std::uint32_t chunkId = 0;
      if (!reader_.readU32(chunkId)) {
        return false;
      }
      if (chunkId == kFacade) {
        return true;
      }
      switch (chunkId) {
      case 0x03036000U: {
        std::int32_t ignored = 0;
        bool ignoredBoolean = false;
        if (!reader_.readI32(ignored) || !reader_.readBoolean(ignoredBoolean) ||
            !reader_.readBoolean(ignoredBoolean) || !reader_.skip(12U) ||
            !readNodeReferenceArray()) {
          return false;
        }
        break;
      }
      case 0x03036001U: {
        std::string ignored;
        if (!readId(ignored) || !reader_.skip(8U)) {
          return false;
        }
        break;
      }
      case 0x03036002U:
        if (!skipSkippableChunk(chunkId)) {
          return false;
        }
        break;
      case 0x03036003U: {
        std::string ignored;
        if (!readNodeReference() || !readId(ignored) || !reader_.skip(8U)) {
          return false;
        }
        break;
      }
      case 0x03036004U:
        if (!reader_.skip(4U)) {
          return false;
        }
        break;
      case 0x03036005U: {
        std::string ignored;
        if (!readId(ignored)) {
          return false;
        }
        break;
      }
      default:
        if (!skipSkippableChunk(chunkId)) {
          return false;
        }
        break;
      }
    }
  }

  bool readHelperSolid() {
    while (true) {
      std::uint32_t chunkId = 0;
      if (!reader_.readU32(chunkId)) {
        return false;
      }
      if (chunkId == kFacade) {
        return true;
      }
      switch (chunkId) {
      case 0x09005000U:
        if (!reader_.skip(4U)) {
          return false;
        }
        break;
      case 0x0900500EU:
        if (!reader_.skip(64U)) {
          return false;
        }
        break;
      case 0x09005010U:
        if (!readNodeReference()) {
          return false;
        }
        break;
      case 0x09005011U: {
        bool ignored = false;
        bool hasExtra = false;
        if (!reader_.readBoolean(ignored) || !reader_.readBoolean(hasExtra) ||
            (hasExtra && !reader_.readBoolean(ignored)) || !readNodeReference()) {
          return false;
        }
        break;
      }
      case 0x09005012U:
        if (!reader_.skip(1U)) {
          return false;
        }
        break;
      default:
        if (!skipSkippableChunk(chunkId)) {
          return false;
        }
        break;
      }
    }
  }

  bool readHmsItem() {
    while (true) {
      std::uint32_t chunkId = 0;
      if (!reader_.readU32(chunkId)) {
        return false;
      }
      if (chunkId == kFacade) {
        return true;
      }
      switch (chunkId) {
      case 0x06003001U:
        if (!readNodeReference()) {
          return false;
        }
        break;
      case 0x0600300EU:
      case 0x06003010U:
      case 0x06003011U:
        if (!reader_.skip(10U)) {
          return false;
        }
        break;
      default:
        if (!skipSkippableChunk(chunkId)) {
          return false;
        }
        break;
      }
    }
  }

  bool readSceneMobil() {
    while (true) {
      std::uint32_t chunkId = 0;
      if (!reader_.readU32(chunkId)) {
        return false;
      }
      if (chunkId == kFacade) {
        return true;
      }
      switch (chunkId) {
      case 0x0A005001U: {
        std::string ignored;
        if (!readId(ignored)) {
          return false;
        }
        break;
      }
      case 0x0A005002U: {
        bool ignored = false;
        if (!reader_.readBoolean(ignored)) {
          return false;
        }
        break;
      }
      case 0x0A005003U:
      case 0x0A011004U:
      case 0x0A011006U:
        if (!readNodeReference()) {
          return false;
        }
        break;
      case 0x0A005004U:
        if (!reader_.skip(4U)) {
          return false;
        }
        break;
      case 0x0A011003U:
        if (!readNodeReferenceArray()) {
          return false;
        }
        break;
      case 0x0A011005U:
        if (!readHmsItem()) {
          return false;
        }
        break;
      default:
        if (!skipSkippableChunk(chunkId)) {
          return false;
        }
        break;
      }
    }
  }

  bool readIso4(std::optional<std::array<float, 12>>& output) {
    std::array<float, 12> transform{};
    for (float& value : transform) {
      if (!reader_.readFloat(value) || !std::isfinite(value)) {
        return reader_.fail("invalid CGameCtnBlockInfo spawn transform");
      }
    }
    output = transform;
    return true;
  }

  bool readRootChunk(const std::uint32_t chunkId) {
    switch (chunkId) {
    case 0x0301A006U:
      return reader_.skip(4U);
    case 0x0301A007U:
      return reader_.skip(24U);
    case 0x0301A008U:
      return readNodeReference();
    case 0x0301A009U: {
      std::string ignored;
      bool hasIcon = false;
      if (!reader_.readString(ignored) || !reader_.readBoolean(hasIcon) ||
          (hasIcon && !readNodeReference())) {
        return false;
      }
      return readId(ignored);
    }
    case 0x0301A00AU: {
      std::string ignored;
      return readId(ignored);
    }
    case 0x0301A00BU: {
      std::string ignored;
      return readId(ignored) && readId(ignored) && readId(ignored);
    }
    case 0x0304E005U: {
      std::string ignoredId;
      bool ignoredBoolean = false;
      std::int32_t ignored = 0;
      if (!readId(ignoredId) || !reader_.skip(12U) || !reader_.readBoolean(ignoredBoolean) ||
          !reader_.skip(8U) || !readNodeReference() || !readNodeReferenceArray() ||
          !readNodeReferenceArray()) {
        return false;
      }
      for (int group = 0; group < 2; ++group) {
        std::int32_t variantCount = 0;
        if (!reader_.readCount(variantCount, "mobil variant")) {
          return false;
        }
        for (std::int32_t variant = 0; variant < variantCount; ++variant) {
          if (!readNodeReferenceArray()) {
            return false;
          }
        }
      }
      std::uint8_t ignoredByte = 0;
      std::int16_t ignoredShort = 0;
      return reader_.readU8(ignoredByte) && reader_.readI32(ignored) &&
             reader_.readI16(ignoredShort) && reader_.readI16(ignoredShort);
    }
    case 0x0304E009U: {
      bool ignored = false;
      return reader_.readBoolean(ignored);
    }
    case 0x0304E00CU:
      return readIso4(blockInfo_.spawnGround) && readIso4(blockInfo_.spawnAir);
    case 0x0304E00DU: {
      bool ignored = false;
      return reader_.readBoolean(ignored);
    }
    case 0x0304E00EU: {
      std::int32_t waypoint = 0;
      if (!reader_.readI32(waypoint) || waypoint < 0 || waypoint > 5) {
        return reader_.fail("invalid CGameCtnBlockInfo waypoint type");
      }
      blockInfo_.waypointType = static_cast<BlockWaypointType>(waypoint);
      return readNodeReference() && readNodeReference() && readNodeReference();
    }
    case 0x0304E00FU:
      complete_ = true;
      return reader_.readBoolean(blockInfo_.noRespawn);
    default:
      return skipSkippableChunk(chunkId);
    }
  }

  Input& input_;
  Reader reader_;
  const DocumentProbe& document_;
  std::unordered_map<std::int32_t, std::uint32_t> records_;
  std::optional<std::int32_t> idVersion_;
  std::unordered_map<std::uint32_t, std::string> ids_;
  BlockInfoMetadata blockInfo_;
  bool complete_ = false;
};

} // namespace

BlockInfoParseResult parseBlockInfo(Input& input) {
  const DocumentProbeResult probed = probeDocument(input, false);
  if (!probed) {
    BlockInfoParseResult result;
    result.error = probed.error;
    return result;
  }
  BlockInfoParser parser(input, probed.document);
  return parser.run();
}

} // namespace tmnf::gbx
