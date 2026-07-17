#include "tmnf/gbx/challenge.hpp"

#include "tmnf/gbx/body.hpp"
#include "tmnf/gbx/document.hpp"
#include "tmnf/gbx/span_input.hpp"

#include <bit>
#include <limits>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace tmnf::gbx {
namespace {

constexpr std::uint32_t kChallengeClassId = 0x03043000U;
constexpr std::uint32_t kChallengeMetadataChunkId = 0x03043002U;
constexpr std::uint32_t kChallengeIdentityChunkId = 0x03043003U;
constexpr std::uint32_t kVehicleChunkId = 0x0304300DU;
constexpr std::uint32_t kCheckpointChunkId = 0x03043017U;
constexpr std::uint32_t kLapChunkId = 0x03043018U;
constexpr std::uint32_t kBlockDataChunkId = 0x0304301FU;
constexpr std::uint32_t kCollectorListClassId = 0x0301B000U;
constexpr std::uint32_t kCollectorListChunkId = 0x0301B000U;
constexpr std::uint32_t kBlockSkinClassId = 0x03059000U;
constexpr std::uint32_t kEndMarker = 0xFACADE01U;
constexpr std::uint32_t kSkipMarker = 0x534B4950U;
constexpr std::size_t kMaximumCollectionCount = 1U << 20U;
constexpr std::size_t kMaximumStringLength = 1U << 20U;

ChallengeParseResult failed(std::string message) {
  ChallengeParseResult result;
  result.error = std::move(message);
  return result;
}

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
  Reader(const std::span<const std::byte> bytes, const std::size_t position = 0)
      : bytes_(bytes), position_(position) {}

  bool readU8(std::uint8_t& value) {
    if (!has(1)) {
      return fail("challenge read exceeds the decoded body");
    }
    value = std::to_integer<std::uint8_t>(bytes_[position_++]);
    return true;
  }

  bool readU16(std::uint16_t& value) {
    if (!has(2)) {
      return fail("challenge read exceeds the decoded body");
    }
    value = static_cast<std::uint16_t>(std::to_integer<std::uint8_t>(bytes_[position_])) |
            static_cast<std::uint16_t>(std::to_integer<std::uint8_t>(bytes_[position_ + 1U]) << 8U);
    position_ += 2;
    return true;
  }

  bool readU32(std::uint32_t& value) {
    if (!has(4)) {
      return fail("challenge read exceeds the decoded body");
    }
    value = readLeU32(bytes_.data() + position_);
    position_ += 4;
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
      return fail("invalid challenge string length");
    }
    const char* const begin = reinterpret_cast<const char*>(bytes_.data() + position_);
    value.assign(begin, static_cast<std::size_t>(length));
    position_ += static_cast<std::size_t>(length);
    return true;
  }

  bool skip(const std::size_t count) {
    if (!has(count)) {
      return fail("challenge skip exceeds the decoded body");
    }
    position_ += count;
    return true;
  }

  bool fail(std::string message) {
    if (error_.empty()) {
      error_ = std::move(message);
    }
    return false;
  }

  [[nodiscard]] bool has(const std::size_t count) const noexcept {
    return position_ <= bytes_.size() && count <= bytes_.size() - position_;
  }
  [[nodiscard]] std::size_t position() const noexcept { return position_; }
  [[nodiscard]] std::size_t remaining() const noexcept {
    return position_ <= bytes_.size() ? bytes_.size() - position_ : 0;
  }
  [[nodiscard]] const std::string& error() const noexcept { return error_; }

private:
  std::span<const std::byte> bytes_;
  std::size_t position_ = 0;
  std::string error_;
};

struct IdState final {
  std::optional<std::int32_t> version;
  std::unordered_map<std::uint32_t, std::string> dictionary;
};

struct NodeState final {
  std::unordered_map<std::int32_t, ChallengeBlockSkin> blockSkins;
};

class IdReader final {
public:
  IdReader(Reader& reader, IdState& state) : reader_(reader), state_(state) {}

  bool readId(std::string& value) {
    if (!state_.version.has_value()) {
      std::int32_t version = 0;
      if (!reader_.readI32(version)) {
        return false;
      }
      if (version < 3 || version > 3) {
        return reader_.fail("unsupported GBX ID version");
      }
      state_.version = version;
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
    if ((encoded & 0x3FFFFFFFU) != 0) {
      const auto existing = state_.dictionary.find(encoded);
      if (existing == state_.dictionary.end()) {
        return reader_.fail("challenge references an unknown GBX ID dictionary entry");
      }
      value = existing->second;
      return true;
    }
    if (state_.dictionary.size() >= kMaximumCollectionCount || !reader_.readString(value)) {
      return reader_.fail("invalid challenge GBX ID dictionary entry");
    }
    const std::uint32_t key = encoded + static_cast<std::uint32_t>(state_.dictionary.size()) + 1U;
    if (!state_.dictionary.emplace(key, value).second) {
      return reader_.fail("duplicate challenge GBX ID dictionary entry");
    }
    return true;
  }

  bool readIdent(ChallengeIdent& ident) {
    return readId(ident.id) && readId(ident.collection) && readId(ident.author);
  }

private:
  Reader& reader_;
  IdState& state_;
};

bool plausibleNextChunk(const std::uint32_t chunkId) noexcept {
  return chunkId == kEndMarker || (chunkId & 0xFFFFF000U) == kChallengeClassId;
}

bool readCollectorList(Reader& reader, IdReader& ids) {
  std::int32_t nodeIndex = 0;
  if (!reader.readI32(nodeIndex)) {
    return false;
  }
  if (nodeIndex == -1) {
    return true;
  }
  if (nodeIndex < 0) {
    return reader.fail("invalid challenge collector-list node index");
  }
  std::uint32_t classId = 0;
  std::uint32_t chunkId = 0;
  if (!reader.readU32(classId) || classId != kCollectorListClassId || !reader.readU32(chunkId) ||
      chunkId != kCollectorListChunkId) {
    return reader.fail("invalid challenge collector-list node");
  }
  std::int32_t stockCount = 0;
  if (!reader.readI32(stockCount) || stockCount < 0 ||
      static_cast<std::size_t>(stockCount) > kMaximumCollectionCount) {
    return reader.fail("invalid challenge collector stock count");
  }
  for (std::int32_t index = 0; index < stockCount; ++index) {
    ChallengeIdent blockModel;
    std::int32_t count = 0;
    if (!ids.readIdent(blockModel) || !reader.readI32(count) || count < 0) {
      return reader.fail("invalid challenge collector stock entry");
    }
  }
  if (!reader.readU32(chunkId) || chunkId != kEndMarker) {
    return reader.fail("challenge collector-list node is missing its end marker");
  }
  return true;
}

bool seedIdState(const std::span<const std::byte> bodyBytes, IdState& state, std::string& error) {
  if (bodyBytes.size() < 4 || readLeU32(bodyBytes.data()) != kVehicleChunkId) {
    return true;
  }
  Reader reader(bodyBytes, 4);
  IdReader ids(reader, state);
  ChallengeIdent playerModel;
  if (!ids.readIdent(playerModel)) {
    error = reader.error();
    return false;
  }
  std::uint32_t chunkId = 0;
  if (reader.remaining() >= 4U && reader.readU32(chunkId) && chunkId == 0x03043011U &&
      !readCollectorList(reader, ids)) {
    error = reader.error();
    return false;
  }
  return true;
}

bool readPackDesc(Reader& reader, ChallengePackDesc& pack) {
  std::uint8_t version = 0;
  if (!reader.readU8(version) || version > 3U) {
    return reader.fail("unsupported challenge pack-description version");
  }
  if (version >= 3U && !reader.skip(32U)) {
    return false;
  }
  if (!reader.readString(pack.filePath)) {
    return false;
  }
  if (((!pack.filePath.empty() && version >= 1U) || version >= 3U) &&
      !reader.readString(pack.locatorUrl)) {
    return false;
  }
  return true;
}

bool readBlockSkinNode(Reader& reader, ChallengeBlockSkin& skin) {
  for (;;) {
    std::uint32_t chunkId = 0;
    if (!reader.readU32(chunkId)) {
      return false;
    }
    if (chunkId == kEndMarker) {
      return true;
    }
    if (chunkId == 0x03059000U) {
      std::string unused;
      if (!reader.readString(skin.text) || !reader.readString(unused)) {
        return false;
      }
      continue;
    }
    if (chunkId == 0x03059001U || chunkId == 0x03059002U) {
      ChallengePackDesc pack;
      if (!reader.readString(skin.text) || !readPackDesc(reader, pack)) {
        return false;
      }
      skin.pack = std::move(pack);
      if (chunkId == 0x03059002U) {
        ChallengePackDesc parentPack;
        if (!readPackDesc(reader, parentPack)) {
          return false;
        }
        skin.parentPack = std::move(parentPack);
      }
      continue;
    }
    if (chunkId == 0x03059003U) {
      std::int32_t version = 0;
      ChallengePackDesc foregroundPack;
      if (!reader.readI32(version) || version < 0 || !readPackDesc(reader, foregroundPack)) {
        return reader.fail("invalid challenge foreground skin");
      }
      skin.foregroundPack = std::move(foregroundPack);
      continue;
    }
    return reader.fail("unsupported CGameCtnBlockSkin chunk");
  }
}

bool readBlockSkinRef(Reader& reader, NodeState& state, ChallengeBlock& block) {
  std::int32_t nodeIndex = 0;
  if (!reader.readI32(nodeIndex)) {
    return false;
  }
  if (nodeIndex == -1) {
    return true;
  }
  if (nodeIndex < 0) {
    return reader.fail("invalid challenge block-skin node index");
  }
  const auto existing = state.blockSkins.find(nodeIndex);
  if (existing != state.blockSkins.end()) {
    block.skin = existing->second;
    return true;
  }
  std::uint32_t classId = 0;
  if (!reader.readU32(classId) || classId != kBlockSkinClassId) {
    return reader.fail("challenge block skin has an unexpected class");
  }
  ChallengeBlockSkin skin;
  if (!readBlockSkinNode(reader, skin)) {
    return false;
  }
  state.blockSkins.emplace(nodeIndex, skin);
  block.skin = std::move(skin);
  return true;
}

bool parseBlockDataCandidate(const std::span<const std::byte> bodyBytes,
                             const std::size_t chunkOffset, const IdState& initialState,
                             Challenge& challenge, std::string& error) {
  Reader reader(bodyBytes, chunkOffset + 4U);
  IdState state = initialState;
  NodeState nodeState;
  IdReader ids(reader, state);
  Challenge parsed;
  parsed.blockDataChunkOffset = chunkOffset;

  if (!ids.readIdent(parsed.mapInfo) || !reader.readString(parsed.mapName) ||
      !ids.readIdent(parsed.decoration)) {
    error = reader.error();
    return false;
  }
  for (std::int32_t& dimension : parsed.size) {
    if (!reader.readI32(dimension) || dimension <= 0 || dimension > 1024) {
      error = "invalid challenge map dimensions";
      return false;
    }
  }
  std::uint32_t needUnlock = 0;
  if (!reader.readU32(needUnlock) || needUnlock > 1U || !reader.readI32(parsed.blockDataVersion)) {
    error = "invalid challenge block-data metadata";
    return false;
  }
  parsed.needUnlock = needUnlock != 0;
  if (parsed.blockDataVersion < 0 || parsed.blockDataVersion > 1) {
    error = "unsupported challenge block-data version";
    return false;
  }

  std::int32_t blockCount = 0;
  const std::size_t minimumBlockSize = parsed.blockDataVersion == 0 ? 10U : 12U;
  if (!reader.readI32(blockCount) || blockCount < 0 ||
      static_cast<std::size_t>(blockCount) > kMaximumCollectionCount ||
      static_cast<std::size_t>(blockCount) > reader.remaining() / minimumBlockSize) {
    error = "invalid challenge block count";
    return false;
  }
  parsed.blocks.reserve(static_cast<std::size_t>(blockCount));
  for (std::int32_t index = 0; index < blockCount; ++index) {
    ChallengeBlock block;
    if (!ids.readId(block.name) || block.name.empty() || !reader.readU8(block.direction) ||
        block.direction > 3U) {
      error = reader.error().empty() ? "invalid challenge block record" : reader.error();
      return false;
    }
    for (std::uint8_t& coordinate : block.coordinate) {
      if (!reader.readU8(coordinate)) {
        error = reader.error();
        return false;
      }
    }
    if (parsed.blockDataVersion == 0) {
      std::uint16_t flags = 0;
      if (!reader.readU16(flags)) {
        error = reader.error();
        return false;
      }
      block.flags = flags;
    } else if (!reader.readU32(block.flags)) {
      error = reader.error();
      return false;
    }
    if ((block.flags & (1U << 15U)) != 0 &&
        (!ids.readId(block.author) || !readBlockSkinRef(reader, nodeState, block))) {
      error = reader.error().empty() ? "invalid skinned challenge block" : reader.error();
      return false;
    }
    parsed.blocks.push_back(std::move(block));
  }

  parsed.blockDataEndOffset = reader.position();
  if (!reader.readU32(parsed.nextChunkId) || !plausibleNextChunk(parsed.nextChunkId)) {
    error = "challenge block data does not end at a top-level chunk boundary";
    return false;
  }
  challenge = std::move(parsed);
  return true;
}

bool readCheckpoints(const std::span<const std::byte> bodyBytes, const std::size_t endOffset,
                     std::vector<std::array<std::int32_t, 3>>& checkpoints, std::string& error) {
  std::optional<std::vector<std::array<std::int32_t, 3>>> match;
  for (std::size_t offset = 0; offset + 12U <= endOffset; ++offset) {
    if (readLeU32(bodyBytes.data() + offset) != kCheckpointChunkId ||
        readLeU32(bodyBytes.data() + offset + 4U) != kSkipMarker) {
      continue;
    }
    Reader header(bodyBytes, offset + 8U);
    std::int32_t size = 0;
    if (!header.readI32(size) || size < 4 || static_cast<std::size_t>(size) > header.remaining() ||
        offset + 12U + static_cast<std::size_t>(size) > endOffset) {
      error = "invalid challenge checkpoint chunk size";
      return false;
    }
    Reader payload(bodyBytes.subspan(offset + 12U, static_cast<std::size_t>(size)));
    std::int32_t count = 0;
    if (!payload.readI32(count) || count < 0 ||
        static_cast<std::size_t>(count) > kMaximumCollectionCount ||
        static_cast<std::size_t>(size) != 4U + static_cast<std::size_t>(count) * 12U) {
      error = "invalid challenge checkpoint count";
      return false;
    }
    std::vector<std::array<std::int32_t, 3>> candidate(static_cast<std::size_t>(count));
    for (auto& checkpoint : candidate) {
      for (std::int32_t& coordinate : checkpoint) {
        if (!payload.readI32(coordinate)) {
          error = payload.error();
          return false;
        }
      }
    }
    if (match.has_value()) {
      error = "decoded challenge contains ambiguous checkpoint chunks";
      return false;
    }
    match = std::move(candidate);
  }
  if (match.has_value()) {
    checkpoints = std::move(*match);
  }
  return true;
}

bool readLapMetadata(const std::span<const std::byte> bodyBytes, const std::size_t endOffset,
                     bool& isLapRace, std::int32_t& nbLaps, std::string& error) {
  bool found = false;
  for (std::size_t offset = 0; offset + 12U <= endOffset; ++offset) {
    if (readLeU32(bodyBytes.data() + offset) != kLapChunkId ||
        readLeU32(bodyBytes.data() + offset + 4U) != kSkipMarker) {
      continue;
    }
    if (readLeU32(bodyBytes.data() + offset + 8U) != 8U || offset + 20U > endOffset) {
      error = "invalid challenge lap chunk size";
      return false;
    }
    Reader payload(bodyBytes, offset + 12U);
    std::uint32_t encodedLapRace = 0;
    std::int32_t candidateNbLaps = 0;
    if (!payload.readU32(encodedLapRace) || encodedLapRace > 1U ||
        !payload.readI32(candidateNbLaps) || (encodedLapRace != 0U && candidateNbLaps <= 0)) {
      error = "invalid challenge lap metadata";
      return false;
    }
    if (found) {
      error = "decoded challenge contains ambiguous lap chunks";
      return false;
    }
    found = true;
    isLapRace = encodedLapRace != 0U;
    nbLaps = candidateNbLaps;
  }
  return true;
}

ChallengeHeaderParseResult headerFailed(std::string message) {
  ChallengeHeaderParseResult result;
  result.error = std::move(message);
  return result;
}

bool assignMedalTime(const std::int32_t value, std::optional<std::int32_t>& output) {
  if (value == -1) {
    output.reset();
    return true;
  }
  if (value <= 0) {
    return false;
  }
  output = value;
  return true;
}

} // namespace

ChallengeHeaderParseResult
parseChallengeHeaderMetadata(const std::span<const std::byte> documentBytes) {
  const ParseResult header = parseHeader(documentBytes);
  if (!header) {
    return headerFailed("unable to parse challenge header: " + header.error);
  }
  if (header.header.classId != kChallengeClassId) {
    return headerFailed("GBX document is not a CGameCtnChallenge");
  }

  const UserDataChunk* metadataChunk = nullptr;
  const UserDataChunk* identityChunk = nullptr;
  for (const UserDataChunk& chunk : header.header.userDataChunks) {
    const UserDataChunk** destination = nullptr;
    if (chunk.id == kChallengeMetadataChunkId) {
      destination = &metadataChunk;
    } else if (chunk.id == kChallengeIdentityChunkId) {
      destination = &identityChunk;
    }
    if (destination != nullptr) {
      if (*destination != nullptr) {
        return headerFailed("challenge header contains duplicate selector metadata");
      }
      *destination = &chunk;
    }
  }
  if (metadataChunk == nullptr || identityChunk == nullptr || metadataChunk->heavy ||
      identityChunk->heavy || metadataChunk->size != 45U ||
      metadataChunk->payloadOffset + metadataChunk->size > documentBytes.size() ||
      identityChunk->payloadOffset + identityChunk->size > documentBytes.size()) {
    return headerFailed("challenge selector metadata is missing or malformed");
  }

  ChallengeHeaderParseResult result;
  Reader metadata(documentBytes.subspan(metadataChunk->payloadOffset, metadataChunk->size));
  std::uint8_t metadataVersion = 0;
  std::uint32_t encodedNeedUnlock = 0;
  std::int32_t bronze = 0;
  std::int32_t silver = 0;
  std::int32_t gold = 0;
  std::int32_t author = 0;
  std::uint32_t encodedLapRace = 0;
  std::uint32_t encodedHasClones = 0;
  if (!metadata.readU8(metadataVersion) || metadataVersion != 11U ||
      !metadata.readU32(encodedNeedUnlock) || encodedNeedUnlock > 1U || !metadata.readI32(bronze) ||
      !metadata.readI32(silver) || !metadata.readI32(gold) || !metadata.readI32(author) ||
      !metadata.readI32(result.metadata.cost) || !metadata.readU32(encodedLapRace) ||
      encodedLapRace > 1U || !metadata.readI32(result.metadata.playMode) ||
      !metadata.readU32(encodedHasClones) || encodedHasClones > 1U ||
      !metadata.readI32(result.metadata.authorScore) ||
      !metadata.readI32(result.metadata.editorMode) || metadata.remaining() != 0U ||
      !assignMedalTime(bronze, result.metadata.medalTimes.bronzeMilliseconds) ||
      !assignMedalTime(silver, result.metadata.medalTimes.silverMilliseconds) ||
      !assignMedalTime(gold, result.metadata.medalTimes.goldMilliseconds) ||
      !assignMedalTime(author, result.metadata.medalTimes.authorMilliseconds)) {
    return headerFailed("unsupported or invalid challenge metadata chunk");
  }
  result.metadata.needUnlock = encodedNeedUnlock != 0U;
  result.metadata.isLapRace = encodedLapRace != 0U;
  result.metadata.hasClones = encodedHasClones != 0U;

  Reader identity(documentBytes.subspan(identityChunk->payloadOffset, identityChunk->size));
  std::uint8_t identityVersion = 0;
  IdState idState;
  IdReader ids(identity, idState);
  if (!identity.readU8(identityVersion) || identityVersion != 5U ||
      !ids.readIdent(result.metadata.mapInfo) || !identity.readString(result.metadata.mapName) ||
      !identity.readU8(result.metadata.mapKind) || result.metadata.mapInfo.id.empty() ||
      result.metadata.mapInfo.collection.empty() || result.metadata.mapName.empty()) {
    return headerFailed("unsupported or invalid challenge identity chunk");
  }
  return result;
}

ChallengeParseResult parseChallengeBody(const std::span<const std::byte> bodyBytes) {
  if (bodyBytes.size() < 8) {
    return failed("decoded challenge body is too small");
  }
  IdState initialState;
  std::string seedError;
  if (!seedIdState(bodyBytes, initialState, seedError)) {
    return failed(seedError.empty() ? "unable to initialize challenge GBX IDs" : seedError);
  }

  std::optional<Challenge> match;
  std::string candidateError;
  std::size_t candidateCount = 0;
  for (std::size_t offset = 0; offset + 4U <= bodyBytes.size(); ++offset) {
    if (readLeU32(bodyBytes.data() + offset) != kBlockDataChunkId) {
      continue;
    }
    ++candidateCount;
    Challenge candidate;
    std::string error;
    if (!parseBlockDataCandidate(bodyBytes, offset, initialState, candidate, error)) {
      if (candidateError.empty()) {
        candidateError = std::move(error);
      }
      continue;
    }
    if (match.has_value()) {
      return failed("decoded challenge contains ambiguous block-data chunks");
    }
    match = std::move(candidate);
  }
  if (!match.has_value()) {
    if (candidateCount == 0) {
      return failed("decoded challenge is missing chunk 0x0304301F");
    }
    return failed("unable to parse challenge block-data chunk: " + candidateError);
  }
  std::string checkpointError;
  if (!readCheckpoints(bodyBytes, match->blockDataChunkOffset, match->checkpoints,
                       checkpointError)) {
    return failed(checkpointError);
  }
  std::string lapError;
  if (!readLapMetadata(bodyBytes, match->blockDataChunkOffset, match->isLapRace, match->nbLaps,
                       lapError)) {
    return failed(lapError);
  }
  ChallengeParseResult result;
  result.challenge = std::move(*match);
  return result;
}

ChallengeParseResult parseChallenge(const std::span<const std::byte> documentBytes) {
  const ChallengeHeaderParseResult headerMetadata = parseChallengeHeaderMetadata(documentBytes);
  if (!headerMetadata) {
    return failed(headerMetadata.error);
  }
  SpanInput input(documentBytes);
  const DocumentProbeResult probed = probeDocument(input, false);
  if (!probed) {
    return failed("unable to probe challenge GBX: " + probed.error);
  }
  if (probed.document.header.classId != kChallengeClassId) {
    return failed("GBX document is not a CGameCtnChallenge");
  }
  const BodyDecodeResult decoded = decodeBody(documentBytes, probed.document);
  if (!decoded) {
    return failed("unable to decode challenge body: " + decoded.error);
  }
  ChallengeParseResult result = parseChallengeBody(decoded.data);
  if (!result) {
    return result;
  }
  if (result.challenge.mapInfo.id != headerMetadata.metadata.mapInfo.id ||
      result.challenge.mapInfo.collection != headerMetadata.metadata.mapInfo.collection ||
      result.challenge.mapInfo.author != headerMetadata.metadata.mapInfo.author ||
      result.challenge.mapName != headerMetadata.metadata.mapName ||
      result.challenge.needUnlock != headerMetadata.metadata.needUnlock ||
      result.challenge.isLapRace != headerMetadata.metadata.isLapRace) {
    return failed("challenge header metadata disagrees with its decoded body");
  }
  result.challenge.medalTimes = headerMetadata.metadata.medalTimes;
  result.challenge.cost = headerMetadata.metadata.cost;
  result.challenge.authorScore = headerMetadata.metadata.authorScore;
  return result;
}

} // namespace tmnf::gbx
