#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace tmnf::gbx {

struct UserDataChunk final {
  std::uint32_t id = 0;
  std::uint32_t size = 0;
  std::size_t payloadOffset = 0;
  bool heavy = false;
};

struct Header final {
  std::uint16_t version = 0;
  char format = '\0';
  char referenceTableCompression = '\0';
  char bodyCompression = '\0';
  char unknownMarker = '\0';
  std::uint32_t classId = 0;
  std::uint32_t userDataSize = 0;
  std::size_t bodyOffset = 0;
  std::vector<UserDataChunk> userDataChunks;
};

struct ParseResult final {
  Header header;
  std::string error;

  [[nodiscard]] explicit operator bool() const noexcept { return error.empty(); }
};

[[nodiscard]] ParseResult parseHeader(std::span<const std::byte> bytes);

} // namespace tmnf::gbx
