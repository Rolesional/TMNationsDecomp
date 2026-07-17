#include "tmnf/gbx/header.hpp"

#include <array>
#include <limits>

namespace tmnf::gbx {
namespace {

class Reader final {
public:
  explicit Reader(const std::span<const std::byte> bytes) : bytes_(bytes) {}

  bool readByte(char& value) {
    if (!has(1)) {
      return false;
    }
    value = static_cast<char>(std::to_integer<unsigned char>(bytes_[position_++]));
    return true;
  }

  bool readU16(std::uint16_t& value) {
    if (!has(2)) {
      return false;
    }
    value = static_cast<std::uint16_t>(std::to_integer<unsigned char>(bytes_[position_])) |
            static_cast<std::uint16_t>(std::to_integer<unsigned char>(bytes_[position_ + 1]) << 8U);
    position_ += 2;
    return true;
  }

  bool readU32(std::uint32_t& value) {
    if (!has(4)) {
      return false;
    }
    value = 0;
    for (std::size_t index = 0; index < 4; ++index) {
      value |= static_cast<std::uint32_t>(std::to_integer<unsigned char>(bytes_[position_ + index]))
               << (index * 8U);
    }
    position_ += 4;
    return true;
  }

  [[nodiscard]] std::size_t position() const noexcept { return position_; }

private:
  [[nodiscard]] bool has(const std::size_t count) const noexcept {
    return count <= bytes_.size() - position_;
  }

  std::span<const std::byte> bytes_;
  std::size_t position_ = 0;
};

ParseResult fail(const char* message) {
  ParseResult result;
  result.error = message;
  return result;
}

} // namespace

ParseResult parseHeader(const std::span<const std::byte> bytes) {
  if (bytes.size() < 3 || std::to_integer<char>(bytes[0]) != 'G' ||
      std::to_integer<char>(bytes[1]) != 'B' || std::to_integer<char>(bytes[2]) != 'X') {
    return fail("missing GBX magic");
  }

  Reader reader(bytes.subspan(3));
  ParseResult result;
  Header& header = result.header;

  if (!reader.readU16(header.version) || !reader.readByte(header.format)) {
    return fail("truncated GBX preamble");
  }
  if (header.version < 6) {
    return fail("GBX versions below 6 are not implemented yet");
  }
  if (header.format != 'B') {
    return fail("only binary GBX files are implemented");
  }
  if (!reader.readByte(header.referenceTableCompression) ||
      !reader.readByte(header.bodyCompression) || !reader.readByte(header.unknownMarker) ||
      !reader.readU32(header.classId) || !reader.readU32(header.userDataSize)) {
    return fail("truncated GBX version 6 header");
  }

  const std::size_t absoluteUserDataStart = 3 + reader.position();
  if (header.userDataSize > bytes.size() - absoluteUserDataStart) {
    return fail("GBX user-data section extends beyond the file");
  }
  header.bodyOffset = absoluteUserDataStart + header.userDataSize;

  if (header.userDataSize == 0) {
    return result;
  }

  std::uint32_t chunkCount = 0;
  if (!reader.readU32(chunkCount)) {
    return fail("missing GBX user-data chunk count");
  }
  constexpr std::uint32_t kMaximumReasonableChunkCount = 1U << 20U;
  if (chunkCount > kMaximumReasonableChunkCount) {
    return fail("unreasonable GBX user-data chunk count");
  }
  if (chunkCount > (header.userDataSize - 4U) / 8U) {
    return fail("GBX user-data chunk table exceeds its declared size");
  }

  header.userDataChunks.reserve(chunkCount);
  for (std::uint32_t index = 0; index < chunkCount; ++index) {
    UserDataChunk chunk;
    std::uint32_t encodedSize = 0;
    if (!reader.readU32(chunk.id) || !reader.readU32(encodedSize)) {
      return fail("truncated GBX user-data chunk table");
    }
    chunk.heavy = (encodedSize & 0x80000000U) != 0;
    chunk.size = encodedSize & 0x7FFFFFFFU;
    header.userDataChunks.push_back(chunk);
  }

  std::uint64_t describedSize = 4U + static_cast<std::uint64_t>(chunkCount) * 8U;
  for (const UserDataChunk& chunk : header.userDataChunks) {
    describedSize += chunk.size;
  }
  if (describedSize != header.userDataSize) {
    return fail("GBX user-data chunks do not match the declared section size");
  }

  std::size_t payloadOffset =
      absoluteUserDataStart + 4U + static_cast<std::size_t>(chunkCount) * 8U;
  for (UserDataChunk& chunk : header.userDataChunks) {
    chunk.payloadOffset = payloadOffset;
    payloadOffset += chunk.size;
  }

  return result;
}

} // namespace tmnf::gbx
