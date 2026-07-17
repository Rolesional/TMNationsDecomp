#include "tmnf/crypto/md5.hpp"
#include "tmnf/gbx/body.hpp"
#include "tmnf/gbx/document.hpp"
#include "tmnf/gbx/span_input.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <span>
#include <vector>

namespace {

std::uint32_t readLeU32(const std::byte* bytes) noexcept {
  std::uint32_t value = 0;
  for (std::size_t index = 0; index < 4; ++index) {
    value |= static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(bytes[index]))
             << (index * 8U);
  }
  return value;
}

} // namespace

int main(const int argc, const char* const* argv) {
  if (argc != 2 && argc != 3) {
    std::cerr << "usage: gbx_body_inspect <file.gbx> [decoded-body-output]\n";
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
  const std::span bytes(reinterpret_cast<const std::byte*>(raw.data()), raw.size());
  tmnf::gbx::SpanInput input(bytes);
  const tmnf::gbx::DocumentProbeResult probed = tmnf::gbx::probeDocument(input, false);
  if (!probed) {
    std::cerr << "unable to probe GBX document: " << probed.error << '\n';
    return 1;
  }
  const tmnf::gbx::BodyDecodeResult decoded = tmnf::gbx::decodeBody(bytes, probed.document);
  if (!decoded) {
    std::cerr << "unable to decode GBX body: " << decoded.error << '\n';
    return 1;
  }
  if (argc == 3) {
    const std::filesystem::path outputPath(argv[2]);
    if (outputPath.has_parent_path()) {
      std::filesystem::create_directories(outputPath.parent_path());
    }
    std::ofstream output(outputPath, std::ios::binary);
    output.write(reinterpret_cast<const char*>(decoded.data.data()),
                 static_cast<std::streamsize>(decoded.data.size()));
    if (!output) {
      std::cerr << "unable to write decoded body: " << outputPath << '\n';
      return 2;
    }
  }

  std::cout << "{\n"
            << "  \"compression\": \"" << probed.document.header.bodyCompression << "\",\n"
            << "  \"size\": " << decoded.data.size() << ",\n"
            << "  \"md5\": \"" << tmnf::crypto::toHex(tmnf::crypto::md5(decoded.data))
            << "\",\n  \"firstChunkId\": ";
  if (decoded.data.size() < 4) {
    std::cout << "null";
  } else {
    std::cout << "\"0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0')
              << readLeU32(decoded.data.data()) << std::dec << "\"";
  }
  std::cout << "\n}\n";
  return 0;
}
