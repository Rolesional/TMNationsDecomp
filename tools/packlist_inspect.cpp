#include "tmnf/crypto/md5.hpp"
#include "tmnf/pak/pack_list.hpp"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <span>
#include <vector>

int main(const int argc, const char* const* argv) {
  if (argc != 2) {
    std::cerr << "usage: packlist_inspect <packlist.dat>\n";
    return 2;
  }

  const std::filesystem::path path(argv[1]);
  std::ifstream stream(path, std::ios::binary);
  if (!stream) {
    std::cerr << "unable to open: " << path << '\n';
    return 2;
  }
  const std::vector<char> raw((std::istreambuf_iterator<char>(stream)),
                              std::istreambuf_iterator<char>());
  const auto bytes = std::span(reinterpret_cast<const std::byte*>(raw.data()), raw.size());
  const tmnf::pak::PackListParseResult parsed = tmnf::pak::parsePackList(bytes);
  if (!parsed) {
    std::cerr << "invalid packlist: " << parsed.error << '\n';
    return 1;
  }

  const tmnf::pak::PackList& packList = parsed.packList;
  std::cout << "{\n"
            << "  \"path\": \"" << path.generic_string() << "\",\n"
            << "  \"version\": " << static_cast<unsigned int>(packList.version) << ",\n"
            << "  \"crc32\": " << packList.crc32 << ",\n"
            << "  \"salt\": " << packList.salt << ",\n"
            << "  \"packs\": [\n";
  for (std::size_t index = 0; index < packList.entries.size(); ++index) {
    const tmnf::pak::PackListEntry& entry = packList.entries[index];
    std::cout << "    {\"name\": \"" << entry.name
              << "\", \"flags\": " << static_cast<unsigned int>(entry.flags) << ", \"baseKey\": \""
              << tmnf::crypto::toHex(entry.baseKey) << "\", \"archiveKey\": \""
              << tmnf::crypto::toHex(tmnf::pak::deriveArchiveKey(entry.baseKey)) << "\"}"
              << (index + 1U == packList.entries.size() ? "" : ",") << '\n';
  }
  std::cout << "  ]\n}\n";
  return 0;
}
