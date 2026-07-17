#include "tmnf/gbx/campaign.hpp"

#include "tmnf/gbx/body.hpp"
#include "tmnf/gbx/document.hpp"
#include "tmnf/gbx/span_input.hpp"

#include <bit>
#include <cstdint>
#include <limits>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace tmnf::gbx {
namespace {

constexpr std::uint32_t kCampaignClassId = 0x03090000U;
constexpr std::uint32_t kChallengeGroupClassId = 0x0308F000U;
constexpr std::uint32_t kGroupNameChunkId = 0x0308F002U;
constexpr std::uint32_t kGroupMapsChunkId = 0x0308F006U;
constexpr std::uint32_t kEndMarker = 0xFACADE01U;
constexpr std::size_t kMaximumCollectionCount = 1U << 20U;
constexpr std::size_t kMaximumStringLength = 1U << 20U;

std::uint32_t readLeU32(const std::byte* const bytes) noexcept {
  std::uint32_t value = 0;
  for (std::size_t index = 0; index < 4; ++index) {
    value |= static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(bytes[index]))
             << (index * 8U);
  }
  return value;
}

class Reader final {
public:
  explicit Reader(const std::span<const std::byte> bytes) : bytes_(bytes) {}

  bool readU32(std::uint32_t& value) {
    if (!has(4U)) {
      return fail("campaign read exceeds the decoded body");
    }
    value = readLeU32(bytes_.data() + position_);
    position_ += 4U;
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
    if (!readI32(length) || length < 0 || static_cast<std::size_t>(length) > kMaximumStringLength ||
        !has(static_cast<std::size_t>(length))) {
      return fail("invalid campaign string length");
    }
    const char* const begin = reinterpret_cast<const char*>(bytes_.data() + position_);
    value.assign(begin, static_cast<std::size_t>(length));
    position_ += static_cast<std::size_t>(length);
    return true;
  }

  bool fail(std::string message) {
    if (error_.empty()) {
      error_ = std::move(message);
    }
    return false;
  }

  [[nodiscard]] const std::string& error() const noexcept { return error_; }

private:
  [[nodiscard]] bool has(const std::size_t count) const noexcept {
    return position_ <= bytes_.size() && count <= bytes_.size() - position_;
  }

  std::span<const std::byte> bytes_;
  std::size_t position_ = 0;
  std::string error_;
};

class IdReader final {
public:
  explicit IdReader(Reader& reader) : reader_(reader) {}

  bool readIdent(ChallengeIdent& ident) {
    return readId(ident.id) && readId(ident.collection) && readId(ident.author);
  }

private:
  bool readId(std::string& value) {
    if (!version_.has_value()) {
      std::int32_t version = 0;
      if (!reader_.readI32(version)) {
        return false;
      }
      if (version != 3) {
        return reader_.fail("unsupported campaign GBX ID version");
      }
      version_ = version;
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
      const auto existing = dictionary_.find(encoded);
      if (existing == dictionary_.end()) {
        return reader_.fail("campaign references an unknown GBX ID dictionary entry");
      }
      value = existing->second;
      return true;
    }
    if (dictionary_.size() >= kMaximumCollectionCount || !reader_.readString(value)) {
      return reader_.fail("invalid campaign GBX ID dictionary entry");
    }
    const std::uint32_t key = encoded + static_cast<std::uint32_t>(dictionary_.size()) + 1U;
    if (!dictionary_.emplace(key, value).second) {
      return reader_.fail("duplicate campaign GBX ID dictionary entry");
    }
    return true;
  }

  Reader& reader_;
  std::optional<std::int32_t> version_;
  std::unordered_map<std::uint32_t, std::string> dictionary_;
};

CampaignParseResult failed(std::string message) {
  CampaignParseResult result;
  result.error = std::move(message);
  return result;
}

} // namespace

CampaignParseResult parseCampaign(const std::span<const std::byte> documentBytes) {
  SpanInput input(documentBytes);
  const DocumentProbeResult probed = probeDocument(input, false);
  if (!probed) {
    return failed("unable to probe campaign GBX: " + probed.error);
  }
  if (probed.document.header.classId != kCampaignClassId) {
    return failed("GBX document is not a CGameCtnCampaign");
  }
  const BodyDecodeResult decoded = decodeBody(documentBytes, probed.document);
  if (!decoded) {
    return failed("unable to decode campaign body: " + decoded.error);
  }

  Reader reader(decoded.data);
  std::uint32_t chunkId = 0;
  std::uint32_t groupEncodingVersion = 0;
  std::int32_t groupCount = 0;
  if (!reader.readU32(chunkId) || chunkId != kCampaignClassId ||
      !reader.readU32(groupEncodingVersion) || groupEncodingVersion != 10U ||
      !reader.readI32(groupCount) || groupCount <= 0 || groupCount > 256) {
    return failed(reader.error().empty() ? "unsupported campaign group-list chunk"
                                         : reader.error());
  }

  CampaignParseResult result;
  result.campaign.groups.reserve(static_cast<std::size_t>(groupCount));
  IdReader ids(reader);
  std::unordered_set<std::int32_t> nodeIndices;
  std::size_t totalMapCount = 0;
  for (std::int32_t groupIndex = 0; groupIndex < groupCount; ++groupIndex) {
    std::int32_t nodeIndex = 0;
    std::uint32_t classId = 0;
    std::uint32_t nameChunkId = 0;
    std::uint32_t mapsChunkId = 0;
    std::uint32_t encodedFlag = 0;
    std::int32_t mapCount = 0;
    CampaignGroup group;
    if (!reader.readI32(nodeIndex) || nodeIndex <= 0 || !nodeIndices.emplace(nodeIndex).second ||
        !reader.readU32(classId) || classId != kChallengeGroupClassId ||
        !reader.readU32(nameChunkId) || nameChunkId != kGroupNameChunkId ||
        !reader.readString(group.name) || group.name.empty() || !reader.readU32(mapsChunkId) ||
        mapsChunkId != kGroupMapsChunkId || !reader.readU32(encodedFlag) || encodedFlag > 1U ||
        !reader.readI32(mapCount) || mapCount < 0 ||
        static_cast<std::size_t>(mapCount) > kMaximumCollectionCount - totalMapCount) {
      return failed(reader.error().empty() ? "invalid campaign challenge group" : reader.error());
    }

    group.maps.reserve(static_cast<std::size_t>(mapCount));
    for (std::int32_t mapIndex = 0; mapIndex < mapCount; ++mapIndex) {
      ChallengeIdent ident;
      if (!ids.readIdent(ident) || ident.id.empty()) {
        return failed(reader.error().empty() ? "invalid campaign map identity" : reader.error());
      }
      group.maps.push_back(std::move(ident));
    }
    std::uint32_t endMarker = 0;
    if (!reader.readU32(endMarker) || endMarker != kEndMarker) {
      return failed(reader.error().empty() ? "campaign challenge group is not terminated"
                                           : reader.error());
    }
    totalMapCount += group.maps.size();
    result.campaign.groups.push_back(std::move(group));
  }

  std::uint32_t nextChunkId = 0;
  if (!reader.readU32(nextChunkId) || (nextChunkId & 0xFFFFF000U) != kCampaignClassId) {
    return failed(reader.error().empty() ? "campaign group list is misaligned" : reader.error());
  }
  return result;
}

} // namespace tmnf::gbx
