#include "tmnf/crypto/md5.hpp"
#include "tmnf/gbx/class_registry.hpp"
#include "tmnf/pak/archive.hpp"
#include "tmnf/pak/pack_list.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace {

bool readFile(const std::filesystem::path& path, std::vector<char>& output) {
  std::ifstream stream(path, std::ios::binary);
  if (!stream) {
    return false;
  }
  output.assign(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
  return true;
}

std::span<const std::byte> asBytes(const std::vector<char>& bytes) {
  return std::span(reinterpret_cast<const std::byte*>(bytes.data()), bytes.size());
}

std::string lowercase(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(), [](const unsigned char character) {
    return static_cast<char>(std::tolower(character));
  });
  return value;
}

std::string jsonEscape(const std::string_view value) {
  std::string result;
  result.reserve(value.size());
  for (const char character : value) {
    if (character == '"' || character == '\\') {
      result.push_back('\\');
      result.push_back(character);
    } else if (static_cast<unsigned char>(character) >= 0x20U) {
      result.push_back(character);
    }
  }
  return result;
}

} // namespace

int main(const int argc, const char* const* argv) {
  if (argc < 3 || argc > 5) {
    std::cerr << "usage: archive_inspect <file.pak> <packlist.dat> [identifier] [--all]\n";
    return 2;
  }

  const std::filesystem::path archivePath(argv[1]);
  const std::filesystem::path packListPath(argv[2]);
  std::optional<std::string> requestedIdentifier;
  bool listAll = false;
  for (int index = 3; index < argc; ++index) {
    if (std::string_view(argv[index]) == "--all") {
      listAll = true;
    } else if (!requestedIdentifier.has_value()) {
      requestedIdentifier = lowercase(argv[index]);
    } else {
      std::cerr << "multiple archive identifiers were supplied\n";
      return 2;
    }
  }
  const std::string identifier =
      requestedIdentifier.value_or(lowercase(archivePath.stem().string()));
  std::vector<char> archiveRaw;
  std::vector<char> packListRaw;
  if (!readFile(archivePath, archiveRaw)) {
    std::cerr << "unable to open: " << archivePath << '\n';
    return 2;
  }
  if (!readFile(packListPath, packListRaw)) {
    std::cerr << "unable to open: " << packListPath << '\n';
    return 2;
  }

  const tmnf::pak::PackListParseResult parsedPackList =
      tmnf::pak::parsePackList(asBytes(packListRaw));
  if (!parsedPackList) {
    std::cerr << "invalid packlist: " << parsedPackList.error << '\n';
    return 1;
  }
  const auto entry =
      std::find_if(parsedPackList.packList.entries.begin(), parsedPackList.packList.entries.end(),
                   [&identifier](const tmnf::pak::PackListEntry& candidate) {
                     return lowercase(candidate.name) == identifier;
                   });
  if (entry == parsedPackList.packList.entries.end()) {
    std::cerr << "packlist has no key for identifier: " << identifier << '\n';
    return 1;
  }

  const tmnf::crypto::Md5Digest key = tmnf::pak::deriveArchiveKey(entry->baseKey);
  const tmnf::pak::ArchiveParseResult parsed = tmnf::pak::parseArchive(asBytes(archiveRaw), key);
  if (!parsed) {
    std::cerr << "invalid archive: " << parsed.error << '\n';
    return 1;
  }
  const tmnf::pak::Archive& archive = parsed.archive;
  const auto compressedCount =
      std::count_if(archive.files.begin(), archive.files.end(),
                    [](const tmnf::pak::FileEntry& file) { return file.compressed(); });
  const auto encryptedCount =
      std::count_if(archive.files.begin(), archive.files.end(),
                    [](const tmnf::pak::FileEntry& file) { return file.encrypted(); });
  const auto dummyWriteCount =
      std::count_if(archive.files.begin(), archive.files.end(),
                    [](const tmnf::pak::FileEntry& file) { return !file.dontUseDummyWrite(); });
  std::cout << "{\n"
            << "  \"path\": \"" << jsonEscape(archivePath.generic_string()) << "\",\n"
            << "  \"identifier\": \"" << jsonEscape(identifier) << "\",\n"
            << "  \"version\": " << archive.header.version << ",\n"
            << "  \"headerMd5\": \"" << tmnf::crypto::toHex(archive.headerMd5) << "\",\n"
            << "  \"gbxHeadersStart\": " << archive.gbxHeadersStart << ",\n"
            << "  \"dataStart\": " << archive.dataStart << ",\n"
            << "  \"gbxHeadersSize\": " << archive.gbxHeadersSize << ",\n"
            << "  \"gbxHeadersCompressedSize\": " << archive.gbxHeadersCompressedSize << ",\n"
            << "  \"flags\": " << archive.flags << ",\n"
            << "  \"folderCount\": " << archive.folders.size() << ",\n"
            << "  \"fileCount\": " << archive.files.size() << ",\n"
            << "  \"compressedFileCount\": " << compressedCount << ",\n"
            << "  \"encryptedFileCount\": " << encryptedCount << ",\n"
            << "  \"dummyWriteFileCount\": " << dummyWriteCount << ",\n"
            << "  \"encryptedHeaderBytesConsumed\": " << archive.encryptedHeaderBytesConsumed
            << ",\n"
            << "  \"" << (listAll ? "files" : "sampleFiles") << "\": [\n";
  const std::size_t sampleCount =
      listAll ? archive.files.size() : std::min<std::size_t>(archive.files.size(), 12);
  for (std::size_t index = 0; index < sampleCount; ++index) {
    const tmnf::pak::FileEntry& file = archive.files[index];
    std::cout << "    {\"path\": \"" << jsonEscape(file.path) << "\", \"classId\": \"0x" << std::hex
              << std::uppercase << std::setw(8) << std::setfill('0') << file.classId << std::dec
              << "\", \"className\": \"" << tmnf::gbx::className(file.classId).value_or("<unknown>")
              << "\", \"offset\": " << file.offset
              << ", \"uncompressedSize\": " << file.uncompressedSize
              << ", \"compressedSize\": " << file.compressedSize << ", \"flags\": \"0x" << std::hex
              << std::uppercase << std::setw(16) << std::setfill('0') << file.flags << std::dec
              << "\"}" << (index + 1U == sampleCount ? "" : ",") << '\n';
  }
  std::cout << "  ]\n}\n";
  return 0;
}
