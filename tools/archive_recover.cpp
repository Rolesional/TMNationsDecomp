#include "tmnf/pak/archive.hpp"
#include "tmnf/pak/cipher_recovery.hpp"
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
    std::cerr << "usage: archive_recover <file.pak> <packlist.dat> <entry> <output> "
                 "[identifier]\n";
    return 2;
  }

  const std::filesystem::path archivePath(argv[1]);
  const std::string requestedPath = normalized(argv[3]);
  const std::filesystem::path outputPath(argv[4]);
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

  std::cout << "entry flags=0x" << std::hex << std::uppercase << std::setw(16) << std::setfill('0')
            << file->flags << std::dec
            << " dummyWrite=" << (file->dontUseDummyWrite() ? "disabled" : "enabled") << '\n';

  tmnf::pak::CipherRecoveryResult recovered =
      tmnf::pak::recoverFileDataByDeflateOracle(asBytes(archiveRaw), archive.archive, *file, key);
  if (!recovered) {
    std::cerr << "unable to recover archive entry: " << recovered.error
              << " (candidates=" << recovered.candidateCount
              << ", explored=" << recovered.exploredStateCount << ")\n";
    return 1;
  }
  if (outputPath.has_parent_path()) {
    std::filesystem::create_directories(outputPath.parent_path());
  }
  std::ofstream output(outputPath, std::ios::binary);
  output.write(reinterpret_cast<const char*>(recovered.data.data()),
               static_cast<std::streamsize>(recovered.data.size()));
  if (!output) {
    std::cerr << "unable to write: " << outputPath << '\n';
    return 2;
  }

  const std::size_t correctedCycles =
      std::count_if(recovered.cycleXors.begin(), recovered.cycleXors.end(),
                    [](const std::uint64_t value) { return value != 0; });
  std::cout << "recovered " << recovered.data.size() << " bytes using " << correctedCycles << '/'
            << recovered.cycleXors.size() << " corrected cycles from " << recovered.candidateCount
            << " candidates (explored " << recovered.exploredStateCount << ", uniqueness "
            << (recovered.uniquenessProven ? "proven" : "not proven") << ")\n";
  for (std::size_t index = 0; index < recovered.cycleXors.size(); ++index) {
    if (recovered.cycleXors[index] != 0) {
      std::cout << "cycle " << index << ": 0x" << std::hex << std::uppercase << std::setw(16)
                << std::setfill('0') << recovered.cycleXors[index] << std::dec << '\n';
    }
  }
  return 0;
}
