#pragma once

#include "tmnf/crypto/md5.hpp"
#include "tmnf/pak/header.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace tmnf::pak {

struct Folder final {
  std::int32_t parentIndex = -1;
  std::string name;
};

struct FileEntry final {
  std::int32_t folderIndex = -1;
  std::string name;
  std::string path;
  std::int32_t unknown = 0;
  std::int32_t uncompressedSize = 0;
  std::int32_t compressedSize = 0;
  std::uint32_t offset = 0;
  std::uint32_t classId = 0;
  std::uint64_t flags = 0;

  [[nodiscard]] bool compressed() const noexcept { return (flags & 0x3cU) != 0; }
  [[nodiscard]] bool dontUseDummyWrite() const noexcept {
    return (flags & 0x0000000100000000ULL) != 0;
  }
  [[nodiscard]] bool publicFile() const noexcept { return (flags & 0x0002000000000000ULL) != 0; }
  [[nodiscard]] bool forceNoCrypt() const noexcept { return (flags & 0x0004000000000000ULL) != 0; }
  [[nodiscard]] bool encrypted() const noexcept { return !publicFile() && !forceNoCrypt(); }
};

struct Archive final {
  Header header;
  crypto::Md5Digest headerMd5{};
  std::uint32_t gbxHeadersStart = 0;
  std::int32_t dataStart = 0;
  std::int32_t gbxHeadersSize = 0;
  std::int32_t gbxHeadersCompressedSize = 0;
  std::uint32_t flags = 0;
  std::size_t encryptedHeaderBytesConsumed = 0;
  std::vector<Folder> folders;
  std::vector<FileEntry> files;
};

struct ArchiveParseResult final {
  Archive archive;
  std::string error;

  [[nodiscard]] explicit operator bool() const noexcept { return error.empty(); }
};

[[nodiscard]] ArchiveParseResult parseArchive(std::span<const std::byte> bytes,
                                              const crypto::Md5Digest& archiveKey);

} // namespace tmnf::pak
