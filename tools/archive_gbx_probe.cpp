#include "tmnf/gbx/class_registry.hpp"
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

bool isGbxPath(const std::string_view path) {
  if (path.size() < 4) {
    return false;
  }
  return lowercase(std::string(path.substr(path.size() - 4))) == ".gbx";
}

class SuppressInitializerInput final : public tmnf::gbx::Input {
public:
  explicit SuppressInitializerInput(tmnf::gbx::Input& input) : input_(input) {}

  bool readBytes(const std::span<std::byte> destination) override {
    return input_.readBytes(destination);
  }
  void initialize(std::span<const std::byte>) noexcept override {}
  [[nodiscard]] std::string_view error() const noexcept override { return input_.error(); }
  [[nodiscard]] std::size_t position() const noexcept override { return input_.position(); }
  [[nodiscard]] std::size_t size() const noexcept override { return input_.size(); }

private:
  tmnf::gbx::Input& input_;
};

} // namespace

int main(const int argc, const char* const* argv) {
  if (argc != 3 && argc != 4) {
    std::cerr << "usage: archive_gbx_probe <file.pak> <packlist.dat> [identifier]\n";
    return 2;
  }

  const std::filesystem::path archivePath(argv[1]);
  const std::filesystem::path packListPath(argv[2]);
  const std::string identifier =
      argc == 4 ? lowercase(argv[3]) : lowercase(archivePath.stem().string());
  std::vector<char> archiveRaw;
  std::vector<char> packListRaw;
  if (!readFile(archivePath, archiveRaw) || !readFile(packListPath, packListRaw)) {
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
                     return lowercase(entry.name) == identifier;
                   });
  if (keyEntry == packList.packList.entries.end()) {
    std::cerr << "packlist has no key for identifier: " << identifier << '\n';
    return 1;
  }

  const tmnf::crypto::Md5Digest key = tmnf::pak::deriveArchiveKey(keyEntry->baseKey);
  const tmnf::pak::ArchiveParseResult parsedArchive =
      tmnf::pak::parseArchive(asBytes(archiveRaw), key);
  if (!parsedArchive) {
    std::cerr << "invalid archive: " << parsedArchive.error << '\n';
    return 1;
  }

  std::size_t gbxCount = 0;
  std::size_t preludeCount = 0;
  std::size_t fullyConsumedCount = 0;
  std::size_t rootSeedRequiredCount = 0;
  for (const tmnf::pak::FileEntry& file : parsedArchive.archive.files) {
    if (!isGbxPath(file.path)) {
      continue;
    }
    ++gbxCount;
    tmnf::pak::FileDataReader preludeReader(asBytes(archiveRaw), parsedArchive.archive, file, key);
    if (!preludeReader) {
      std::cout << "FAIL\t" << file.path << '\t' << file.uncompressedSize << '\t'
                << preludeReader.error() << '\n';
      continue;
    }
    const tmnf::gbx::DocumentProbeResult prelude = tmnf::gbx::probeDocument(preludeReader, false);
    if (!prelude) {
      std::cout << "FAIL\t" << file.path << '\t' << file.uncompressedSize << '\t' << prelude.error
                << '\n';
      continue;
    }
    ++preludeCount;

    tmnf::pak::FileDataReader reader(asBytes(archiveRaw), parsedArchive.archive, file, key);
    const tmnf::gbx::DocumentProbeResult probe = tmnf::gbx::probeDocument(reader, true);
    if (!probe) {
      std::cout << "PARTIAL\t" << file.path << '\t' << file.uncompressedSize << "\t0x" << std::hex
                << std::uppercase << std::setw(8) << std::setfill('0')
                << prelude.document.header.classId << std::dec << '\t'
                << prelude.document.header.bodyCompression << '\t';
      if (prelude.document.firstChunkId.has_value()) {
        std::cout << "0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0')
                  << *prelude.document.firstChunkId << std::dec;
      } else {
        std::cout << '-';
      }
      std::cout << '\t' << probe.error << '\n';
      continue;
    }
    if (probe.document.fullyConsumed) {
      ++fullyConsumedCount;
    }
    tmnf::pak::FileDataReader noSeedReader(asBytes(archiveRaw), parsedArchive.archive, file, key);
    SuppressInitializerInput noSeedInput(noSeedReader);
    const bool rootSeedRequired = !tmnf::gbx::probeDocument(noSeedInput, true);
    rootSeedRequiredCount += rootSeedRequired ? 1U : 0U;

    std::cout << "PASS\t" << file.path << '\t' << file.uncompressedSize << "\t0x" << std::hex
              << std::uppercase << std::setw(8) << std::setfill('0')
              << probe.document.header.classId << std::dec << '\t'
              << probe.document.header.bodyCompression << '\t';
    if (probe.document.firstChunkId.has_value()) {
      std::cout << "0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0')
                << *probe.document.firstChunkId << std::dec;
    } else {
      std::cout << '-';
    }
    std::cout << '\t' << (rootSeedRequired ? "required" : "not-required") << '\n';
  }

  std::cerr << "GBX probes: " << preludeCount << '/' << gbxCount
            << " parsed, fully consumed: " << fullyConsumedCount
            << ", root seed required: " << rootSeedRequiredCount << '\n';
  return gbxCount == 0 ? 1 : 0;
}
