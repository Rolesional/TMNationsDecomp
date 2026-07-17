#include "tmnf/gbx/document.hpp"
#include "tmnf/pak/archive.hpp"
#include "tmnf/pak/file_data.hpp"
#include "tmnf/pak/pack_list.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <span>
#include <string>
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

std::string normalized(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(), [](const unsigned char character) {
    return character == '/' ? '\\' : static_cast<char>(std::tolower(character));
  });
  return value;
}

} // namespace

int main(const int argc, const char* const* argv) {
  if (argc != 5 && argc != 6) {
    std::cerr << "usage: archive_gbx_prefix <file.pak> <packlist.dat> <entry> <max-bytes> "
                 "[identifier]\n";
    return 2;
  }

  const std::filesystem::path archivePath(argv[1]);
  const std::string requestedPath = normalized(argv[3]);
  const std::size_t maximumBytes = std::stoull(argv[4]);
  const std::string identifier =
      argc == 6 ? normalized(argv[5]) : normalized(archivePath.stem().string());
  std::vector<char> archiveRaw;
  std::vector<char> packListRaw;
  if (!readFile(archivePath, archiveRaw) || !readFile(argv[2], packListRaw)) {
    std::cerr << "unable to read archive or packlist\n";
    return 2;
  }

  const tmnf::pak::PackListParseResult packList = tmnf::pak::parsePackList(asBytes(packListRaw));
  if (!packList) {
    std::cerr << "invalid packlist: " << packList.error << '\n';
    return 1;
  }
  const auto keyEntry =
      std::find_if(packList.packList.entries.begin(), packList.packList.entries.end(),
                   [&identifier](const tmnf::pak::PackListEntry& entry) {
                     return normalized(entry.name) == identifier;
                   });
  if (keyEntry == packList.packList.entries.end()) {
    std::cerr << "packlist has no key for identifier: " << identifier << '\n';
    return 1;
  }
  const tmnf::crypto::Md5Digest key = tmnf::pak::deriveArchiveKey(keyEntry->baseKey);
  const tmnf::pak::ArchiveParseResult archive = tmnf::pak::parseArchive(asBytes(archiveRaw), key);
  if (!archive) {
    std::cerr << "invalid archive: " << archive.error << '\n';
    return 1;
  }
  const auto file = std::find_if(archive.archive.files.begin(), archive.archive.files.end(),
                                 [&requestedPath](const tmnf::pak::FileEntry& entry) {
                                   return normalized(entry.path) == requestedPath;
                                 });
  if (file == archive.archive.files.end()) {
    std::cerr << "archive entry not found: " << argv[3] << '\n';
    return 1;
  }

  tmnf::pak::FileDataReader reader(asBytes(archiveRaw), archive.archive, *file, key);
  const tmnf::gbx::DocumentProbeResult probe = tmnf::gbx::probeDocument(reader, false);
  if (!probe) {
    std::cerr << "unable to probe GBX: " << probe.error << '\n';
    return 1;
  }
  std::cout << "class=0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0')
            << probe.document.header.classId << " firstChunk=0x" << std::setw(8)
            << probe.document.firstChunkId.value_or(0) << std::dec
            << " bodyOffset=" << probe.document.bodyDataOffset
            << " currentOffset=" << reader.position() << '\n';

  const std::size_t byteCount = std::min(maximumBytes, reader.size() - reader.position());
  for (std::size_t index = 0; index < byteCount; ++index) {
    std::byte value{};
    if (!reader.readBytes(std::span(&value, 1))) {
      std::cerr << "read stopped at logical offset " << reader.position() << ": " << reader.error()
                << '\n';
      return 1;
    }
    if (index % 16 == 0) {
      std::cout << std::hex << std::uppercase << std::setw(8) << std::setfill('0')
                << probe.document.bodyDataOffset + 4 + index << ": ";
    }
    std::cout << std::setw(2) << std::to_integer<unsigned int>(value)
              << (index % 16 == 15 ? '\n' : ' ');
  }
  if (byteCount % 16 != 0) {
    std::cout << '\n';
  }
  return 0;
}
