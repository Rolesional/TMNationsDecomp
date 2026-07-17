#pragma once

#include "tmnf/crypto/md5.hpp"
#include "tmnf/gbx/input.hpp"
#include "tmnf/pak/archive.hpp"
#include "tmnf/pak/pack_list.hpp"

#include <cstddef>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace tmnf::pak {

struct VfsOperationResult final {
  std::string error;

  [[nodiscard]] explicit operator bool() const noexcept { return error.empty(); }
};

struct VfsReadResult final {
  std::vector<std::byte> data;
  std::string archiveIdentifier;
  std::string error;

  [[nodiscard]] explicit operator bool() const noexcept { return error.empty(); }
};

struct VfsOpenResult final {
  std::unique_ptr<gbx::Input> input;
  std::string archiveIdentifier;
  std::string storedPath;
  std::string error;

  [[nodiscard]] explicit operator bool() const noexcept {
    return error.empty() && input != nullptr;
  }
};

// Normalizes GameBox paths for archive lookup: ASCII case-insensitive, either
// slash style, no leading separators, and no duplicate separators.
[[nodiscard]] std::string normalizeArchivePath(std::string_view path);

// Computes the 17-byte/136-bit name used when NadeoPak flattens a logical path segment.
// TMNF paths are ASCII; hashing is case-insensitive and the hex nibbles follow the game order.
[[nodiscard]] std::string nadeoPathHash(std::string_view path);

class VirtualFileSystem final {
public:
  [[nodiscard]] VfsOperationResult initialize(std::span<const std::byte> packListBytes);
  [[nodiscard]] VfsOperationResult mount(std::string_view identifier,
                                         std::vector<std::byte> archiveBytes);
  // The VFS and its mounted archive bytes must outlive the returned streaming input.
  [[nodiscard]] VfsOpenResult openFile(std::string_view path) const;
  [[nodiscard]] VfsReadResult readFile(std::string_view path) const;
  [[nodiscard]] bool contains(std::string_view path) const;
  [[nodiscard]] std::size_t mountCount() const noexcept { return mounts_.size(); }
  [[nodiscard]] std::size_t fileCount() const noexcept;

private:
  struct Mount final {
    std::string identifier;
    std::vector<std::byte> bytes;
    crypto::Md5Digest key{};
    Archive archive;
    std::unordered_map<std::string, std::size_t> files;
  };

  [[nodiscard]] static std::optional<std::size_t> resolveFileIndex(const Mount& mount,
                                                                   std::string_view path);

  std::optional<PackList> packList_;
  std::vector<Mount> mounts_;
};

} // namespace tmnf::pak
