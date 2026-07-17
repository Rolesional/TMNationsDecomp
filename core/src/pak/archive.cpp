#include "tmnf/pak/archive.hpp"

#include "tmnf/pak/cipher.hpp"

#include <algorithm>
#include <array>
#include <limits>
#include <string_view>

namespace tmnf::pak {
namespace {

ArchiveParseResult fail(const char* const message) {
  ArchiveParseResult result;
  result.error = message;
  return result;
}

std::string buildPath(const std::int32_t folderIndex, const std::string_view fileName,
                      const std::span<const Folder> folders) {
  std::vector<std::string_view> parts;
  std::int32_t current = folderIndex;
  for (std::size_t depth = 0; current != -1 && depth <= folders.size(); ++depth) {
    if (current < 0 || static_cast<std::size_t>(current) >= folders.size()) {
      return {};
    }
    std::string_view part = folders[static_cast<std::size_t>(current)].name;
    while (!part.empty() && (part.back() == '\\' || part.back() == '/')) {
      part.remove_suffix(1);
    }
    parts.push_back(part);
    current = folders[static_cast<std::size_t>(current)].parentIndex;
  }
  if (current != -1) {
    return {};
  }

  std::string path;
  for (auto iterator = parts.rbegin(); iterator != parts.rend(); ++iterator) {
    if (!iterator->empty()) {
      if (!path.empty()) {
        path.push_back('\\');
      }
      path.append(*iterator);
    }
  }
  if (!path.empty()) {
    path.push_back('\\');
  }
  path.append(fileName);
  return path;
}

} // namespace

ArchiveParseResult parseArchive(const std::span<const std::byte> bytes,
                                const crypto::Md5Digest& archiveKey) {
  const ParseResult parsedHeader = parseHeader(bytes);
  if (!parsedHeader) {
    ArchiveParseResult result;
    result.error = parsedHeader.error;
    return result;
  }

  ArchiveParseResult result;
  Archive& archive = result.archive;
  archive.header = parsedHeader.header;
  CipherReader reader(bytes.subspan(archive.header.encryptedHeaderOffset), archiveKey,
                      archive.header.initializationVector);

  if (!reader.readBytes(archive.headerMd5) || !reader.readU32(archive.gbxHeadersStart) ||
      !reader.readI32(archive.dataStart) || !reader.readI32(archive.gbxHeadersSize) ||
      !reader.readI32(archive.gbxHeadersCompressedSize)) {
    return fail("truncated encrypted NadeoPak header");
  }
  std::array<std::byte, 16> unused{};
  if (!reader.readBytes(unused) || !reader.readU32(archive.flags)) {
    return fail("truncated NadeoPak v3 header fields");
  }
  if (archive.dataStart < 0 || static_cast<std::uint64_t>(archive.dataStart) > bytes.size() ||
      archive.gbxHeadersSize < 0 || archive.gbxHeadersCompressedSize < 0) {
    return fail("invalid NadeoPak header sizes");
  }

  std::int32_t folderCount = 0;
  if (!reader.readI32(folderCount) || folderCount < 0 || folderCount > (1 << 20)) {
    return fail("invalid NadeoPak folder count");
  }
  archive.folders.reserve(static_cast<std::size_t>(folderCount));
  for (std::int32_t index = 0; index < folderCount; ++index) {
    Folder folder;
    if (!reader.readI32(folder.parentIndex) || !reader.readString(folder.name)) {
      return fail("truncated NadeoPak folder table");
    }
    if (folder.parentIndex < -1 || folder.parentIndex >= folderCount) {
      return fail("invalid NadeoPak parent folder index");
    }
    if (folder.name.empty() || (folder.name.back() != '\\' && folder.name.back() != '/')) {
      folder.name.push_back('\\');
    }
    archive.folders.push_back(std::move(folder));
  }

  if (archive.folders.size() > 2U && archive.folders[2].name.size() > 4U) {
    const std::string_view seed = archive.folders[2].name;
    if ((static_cast<unsigned char>(seed[0]) & 0x80U) != 0 ||
        (static_cast<unsigned char>(seed[1]) & 0x80U) != 0 ||
        (static_cast<unsigned char>(seed[2]) & 0x80U) != 0 ||
        (static_cast<unsigned char>(seed[3]) & 0x80U) != 0) {
      return fail("non-ASCII NadeoPak cipher seed is not implemented");
    }
    const std::array seedBytes{
        static_cast<std::byte>(seed[2]),
        std::byte{0},
        static_cast<std::byte>(seed[3]),
        std::byte{0},
    };
    reader.initialize(seedBytes);
  }

  std::int32_t fileCount = 0;
  if (!reader.readI32(fileCount) || fileCount < 0 || fileCount > (1 << 24)) {
    return fail("invalid NadeoPak file count");
  }
  archive.files.reserve(static_cast<std::size_t>(fileCount));
  for (std::int32_t index = 0; index < fileCount; ++index) {
    FileEntry file;
    if (!reader.readI32(file.folderIndex) || !reader.readString(file.name) ||
        !reader.readI32(file.unknown) || !reader.readI32(file.uncompressedSize) ||
        !reader.readI32(file.compressedSize) || !reader.readU32(file.offset) ||
        !reader.readU32(file.classId) || !reader.readU64(file.flags)) {
      return fail("truncated NadeoPak file table");
    }
    if (file.folderIndex < -1 || file.folderIndex >= folderCount || file.uncompressedSize < 0 ||
        file.compressedSize < 0) {
      return fail("invalid NadeoPak file entry");
    }
    file.path = buildPath(file.folderIndex, file.name, archive.folders);
    if (file.path.empty()) {
      return fail("invalid NadeoPak folder hierarchy");
    }
    archive.files.push_back(std::move(file));
  }
  archive.encryptedHeaderBytesConsumed = reader.position();
  return result;
}

} // namespace tmnf::pak
