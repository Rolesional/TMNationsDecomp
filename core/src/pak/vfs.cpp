#include "tmnf/pak/vfs.hpp"

#include "tmnf/pak/file_data.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <utility>

namespace tmnf::pak {
namespace {

VfsOperationResult operationFailed(std::string message) {
  VfsOperationResult result;
  result.error = std::move(message);
  return result;
}

VfsReadResult readFailed(std::string message) {
  VfsReadResult result;
  result.error = std::move(message);
  return result;
}

VfsOpenResult openFailed(std::string message) {
  VfsOpenResult result;
  result.error = std::move(message);
  return result;
}

char normalizedCharacter(const char character) noexcept {
  if (character == '/') {
    return '\\';
  }
  if (character >= 'A' && character <= 'Z') {
    return static_cast<char>(character - 'A' + 'a');
  }
  return character;
}

} // namespace

std::string normalizeArchivePath(const std::string_view path) {
  std::string result;
  result.reserve(path.size());
  for (const char source : path) {
    const char character = normalizedCharacter(source);
    if (character == '\\') {
      if (result.empty() || result.back() == '\\') {
        continue;
      }
    }
    result.push_back(character);
  }
  while (!result.empty() && result.back() == '\\') {
    result.pop_back();
  }
  return result;
}

std::string nadeoPathHash(const std::string_view path) {
  std::string normalized;
  normalized.reserve(path.size());
  for (const char character : path) {
    normalized.push_back(normalizedCharacter(character));
  }
  const crypto::Md5Digest digest = crypto::md5(normalized);
  std::array<std::byte, 17> hash{};
  hash[0] = static_cast<std::byte>(normalized.size() & 0xffU);
  std::copy(digest.begin(), digest.end(), hash.begin() + 1);

  constexpr std::string_view kHex = "0123456789ABCDEF";
  std::string result(hash.size() * 2U, '0');
  for (std::size_t index = 0; index < hash.size(); ++index) {
    const std::uint8_t value = std::to_integer<std::uint8_t>(hash[index]);
    result[index * 2U] = kHex[value & 0x0fU];
    result[index * 2U + 1U] = kHex[value >> 4U];
  }
  return result;
}

VfsOperationResult VirtualFileSystem::initialize(const std::span<const std::byte> packListBytes) {
  const PackListParseResult parsed = parsePackList(packListBytes);
  if (!parsed) {
    return operationFailed("invalid packlist: " + parsed.error);
  }
  packList_ = parsed.packList;
  mounts_.clear();
  return {};
}

VfsOperationResult VirtualFileSystem::mount(const std::string_view identifier,
                                            std::vector<std::byte> archiveBytes) {
  if (!packList_.has_value()) {
    return operationFailed("VFS packlist is not initialized");
  }
  const std::string normalizedIdentifier = normalizeArchivePath(identifier);
  if (normalizedIdentifier.empty() || normalizedIdentifier.find('\\') != std::string::npos) {
    return operationFailed("invalid archive identifier");
  }
  if (std::any_of(mounts_.begin(), mounts_.end(), [&normalizedIdentifier](const Mount& mount) {
        return mount.identifier == normalizedIdentifier;
      })) {
    return operationFailed("archive is already mounted: " + normalizedIdentifier);
  }

  const auto keyEntry =
      std::find_if(packList_->entries.begin(), packList_->entries.end(),
                   [&normalizedIdentifier](const PackListEntry& entry) {
                     return normalizeArchivePath(entry.name) == normalizedIdentifier;
                   });
  if (keyEntry == packList_->entries.end()) {
    return operationFailed("packlist has no key for archive: " + normalizedIdentifier);
  }

  const crypto::Md5Digest key = deriveArchiveKey(keyEntry->baseKey);
  const ArchiveParseResult parsed = parseArchive(archiveBytes, key);
  if (!parsed) {
    return operationFailed("invalid " + normalizedIdentifier + " archive: " + parsed.error);
  }

  Mount mount;
  mount.identifier = normalizedIdentifier;
  mount.bytes = std::move(archiveBytes);
  mount.key = key;
  mount.archive = parsed.archive;
  mount.files.reserve(mount.archive.files.size());
  for (std::size_t index = 0; index < mount.archive.files.size(); ++index) {
    mount.files.try_emplace(normalizeArchivePath(mount.archive.files[index].path), index);
  }
  mounts_.push_back(std::move(mount));
  return {};
}

VfsReadResult VirtualFileSystem::readFile(const std::string_view path) const {
  const std::string normalizedPath = normalizeArchivePath(path);
  if (normalizedPath.empty()) {
    return readFailed("empty archive path");
  }
  for (auto mount = mounts_.rbegin(); mount != mounts_.rend(); ++mount) {
    const std::optional<std::size_t> file = resolveFileIndex(*mount, normalizedPath);
    if (!file.has_value()) {
      continue;
    }
    const FileEntry& entry = mount->archive.files[*file];
    FileDataResult data = readFileData(mount->bytes, mount->archive, entry, mount->key);
    if (!data) {
      return readFailed("unable to read " + mount->identifier + ":" + normalizedPath + ": " +
                        data.error);
    }
    VfsReadResult result;
    result.data = std::move(data.data);
    result.archiveIdentifier = mount->identifier;
    return result;
  }
  return readFailed("archive file not found: " + normalizedPath);
}

VfsOpenResult VirtualFileSystem::openFile(const std::string_view path) const {
  const std::string normalizedPath = normalizeArchivePath(path);
  if (normalizedPath.empty()) {
    return openFailed("empty archive path");
  }
  for (auto mount = mounts_.rbegin(); mount != mounts_.rend(); ++mount) {
    const std::optional<std::size_t> file = resolveFileIndex(*mount, normalizedPath);
    if (!file.has_value()) {
      continue;
    }
    const FileEntry& entry = mount->archive.files[*file];
    auto input = std::make_unique<FileDataReader>(mount->bytes, mount->archive, entry, mount->key);
    if (!*input) {
      return openFailed("unable to open " + mount->identifier + ":" + normalizedPath + ": " +
                        std::string(input->error()));
    }
    VfsOpenResult result;
    result.input = std::move(input);
    result.archiveIdentifier = mount->identifier;
    result.storedPath = entry.path;
    return result;
  }
  return openFailed("archive file not found: " + normalizedPath);
}

bool VirtualFileSystem::contains(const std::string_view path) const {
  const std::string normalizedPath = normalizeArchivePath(path);
  return !normalizedPath.empty() &&
         std::any_of(mounts_.begin(), mounts_.end(), [&normalizedPath](const Mount& mount) {
           return resolveFileIndex(mount, normalizedPath).has_value();
         });
}

std::optional<std::size_t> VirtualFileSystem::resolveFileIndex(const Mount& mount,
                                                               const std::string_view path) {
  const auto resolveCandidate =
      [&mount](const std::string_view candidatePath) -> std::optional<std::size_t> {
    if (const auto exact = mount.files.find(std::string(candidatePath));
        exact != mount.files.end()) {
      return exact->second;
    }

    std::string directory;
    std::string name;
    if (const std::size_t separator = candidatePath.rfind('\\');
        separator != std::string_view::npos) {
      directory = candidatePath.substr(0, separator);
      name = candidatePath.substr(separator + 1U);
    } else {
      name = candidatePath;
    }
    while (!directory.empty()) {
      const std::string hashedPath = directory + "\\" + normalizeArchivePath(nadeoPathHash(name));
      if (const auto hashed = mount.files.find(hashedPath); hashed != mount.files.end()) {
        return hashed->second;
      }
      const std::size_t separator = directory.rfind('\\');
      if (separator == std::string::npos) {
        break;
      }
      name = directory.substr(separator + 1U) + "\\" + name;
      directory.resize(separator);
    }
    return std::nullopt;
  };

  if (const std::optional<std::size_t> direct = resolveCandidate(path); direct.has_value()) {
    return direct;
  }
  if (!path.starts_with(mount.identifier + "\\")) {
    return resolveCandidate(mount.identifier + "\\" + std::string(path));
  }
  return std::nullopt;
}

std::size_t VirtualFileSystem::fileCount() const noexcept {
  std::size_t count = 0;
  for (const Mount& mount : mounts_) {
    count += mount.archive.files.size();
  }
  return count;
}

} // namespace tmnf::pak
