#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>

namespace tmnf::pak {

inline constexpr std::uint32_t kSupportedVersion = 3;
inline constexpr std::size_t kPreambleSize = 12;
inline constexpr std::size_t kInitializationVectorSize = 8;
inline constexpr std::size_t kHeaderSize = kPreambleSize + kInitializationVectorSize;

struct Header final {
  std::uint32_t version = 0;
  std::uint64_t initializationVector = 0;
  std::size_t encryptedHeaderOffset = 0;
};

struct ParseResult final {
  Header header;
  std::string error;

  [[nodiscard]] explicit operator bool() const noexcept { return error.empty(); }
};

// Reproduces the checks performed by TmForever.exe at 0x009020A0 and reads
// the initialization vector consumed immediately afterward.
[[nodiscard]] ParseResult parseHeader(std::span<const std::byte> bytes);

} // namespace tmnf::pak
