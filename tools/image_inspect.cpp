#include "tmnf/crypto/md5.hpp"
#include "tmnf/image/dds.hpp"
#include "tmnf/image/tga.hpp"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <span>
#include <vector>

int main(const int argc, const char* const* argv) {
  if (argc != 2) {
    std::cerr << "usage: image_inspect <file.dds|file.tga>\n";
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
  const bool dds = bytes.size() >= 4 && std::to_integer<char>(bytes[0]) == 'D' &&
                   std::to_integer<char>(bytes[1]) == 'D' &&
                   std::to_integer<char>(bytes[2]) == 'S' && std::to_integer<char>(bytes[3]) == ' ';
  const tmnf::image::DecodeResult decoded =
      dds ? tmnf::image::decodeDds(bytes) : tmnf::image::decodeTga(bytes);
  if (!decoded) {
    std::cerr << "unable to decode image: " << decoded.error << '\n';
    return 1;
  }
  const std::span pixelBytes(reinterpret_cast<const std::byte*>(decoded.image.pixels.data()),
                             decoded.image.pixels.size());
  std::cout << "{\n"
            << "  \"format\": \"" << decoded.format << "\",\n"
            << "  \"width\": " << decoded.image.width << ",\n"
            << "  \"height\": " << decoded.image.height << ",\n"
            << "  \"pixelBytes\": " << decoded.image.pixels.size() << ",\n"
            << "  \"pixelMd5\": \"" << tmnf::crypto::toHex(tmnf::crypto::md5(pixelBytes)) << "\"\n"
            << "}\n";
  return 0;
}
