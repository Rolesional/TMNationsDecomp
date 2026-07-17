#pragma once

#include "tmnf/crypto/md5.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace tmnf::pak {

struct PackListEntry final {
  std::uint8_t flags = 0;
  std::string name;
  crypto::Md5Digest baseKey{};
};

struct PackList final {
  std::uint8_t version = 0;
  std::uint32_t crc32 = 0;
  std::uint32_t salt = 0;
  std::vector<PackListEntry> entries;
  std::array<std::byte, 16> signature{};
};

struct PackListParseResult final {
  PackList packList;
  std::string error;

  [[nodiscard]] explicit operator bool() const noexcept { return error.empty(); }
};

[[nodiscard]] PackListParseResult parsePackList(std::span<const std::byte> bytes);
[[nodiscard]] crypto::Md5Digest deriveArchiveKey(const crypto::Md5Digest& baseKey);

} // namespace tmnf::pak
