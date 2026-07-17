#include "tmnf/pak/header.hpp"

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
    std::cerr << "usage: pak_inspect <file.pak>\n";
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
  const tmnf::pak::ParseResult parsed = tmnf::pak::parseHeader(bytes);
  if (!parsed) {
    std::cerr << "invalid NadeoPak: " << parsed.error << '\n';
    return 1;
  }

  std::cout << "{\n"
            << "  \"path\": \"" << path.generic_string() << "\",\n"
            << "  \"version\": " << parsed.header.version << ",\n"
            << "  \"initializationVector\": \"0x" << std::hex << std::uppercase << std::setw(16)
            << std::setfill('0') << parsed.header.initializationVector << std::dec << "\",\n"
            << "  \"encryptedHeaderOffset\": " << parsed.header.encryptedHeaderOffset << ",\n"
            << "  \"encryptedPayloadSize\": " << bytes.size() - parsed.header.encryptedHeaderOffset
            << "\n"
            << "}\n";
  return 0;
}
