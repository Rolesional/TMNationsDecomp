#include "tmnf/image/dds.hpp"
#include "tmnf/image/tga.hpp"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <span>
#include <vector>

namespace {

void writeU16(std::vector<std::byte>& bytes, const std::size_t offset, const std::uint16_t value) {
  bytes[offset] = static_cast<std::byte>(value & 0xffU);
  bytes[offset + 1U] = static_cast<std::byte>(value >> 8U);
}

void writeU32(std::vector<std::byte>& bytes, const std::size_t offset, const std::uint32_t value) {
  for (std::size_t index = 0; index < 4; ++index) {
    bytes[offset + index] = static_cast<std::byte>((value >> (index * 8U)) & 0xffU);
  }
}

std::vector<std::byte> ddsHeader(const std::uint32_t fourCc, const std::size_t dataSize) {
  std::vector<std::byte> bytes(128U + dataSize);
  bytes[0] = std::byte{'D'};
  bytes[1] = std::byte{'D'};
  bytes[2] = std::byte{'S'};
  bytes[3] = std::byte{' '};
  writeU32(bytes, 4, 124);
  writeU32(bytes, 8, 0x00081007);
  writeU32(bytes, 12, 4);
  writeU32(bytes, 16, 4);
  writeU32(bytes, 76, 32);
  writeU32(bytes, 80, 4);
  writeU32(bytes, 84, fourCc);
  return bytes;
}

constexpr std::uint32_t fourCc(const char a, const char b, const char c, const char d) {
  return static_cast<std::uint32_t>(a) | (static_cast<std::uint32_t>(b) << 8U) |
         (static_cast<std::uint32_t>(c) << 16U) | (static_cast<std::uint32_t>(d) << 24U);
}

bool matchesPixel(const tmnf::image::RgbaImage& image, const std::size_t pixel,
                  const std::uint8_t red, const std::uint8_t green, const std::uint8_t blue,
                  const std::uint8_t alpha) {
  const std::size_t offset = pixel * 4U;
  return image.pixels[offset] == red && image.pixels[offset + 1U] == green &&
         image.pixels[offset + 2U] == blue && image.pixels[offset + 3U] == alpha;
}

bool testBc1() {
  std::vector<std::byte> bytes = ddsHeader(fourCc('D', 'X', 'T', '1'), 8);
  writeU16(bytes, 128, 0xf800);
  writeU16(bytes, 130, 0x07e0);
  writeU32(bytes, 132, 0xe4e4e4e4);
  const tmnf::image::DecodeResult result = tmnf::image::decodeDds(bytes);
  return result && result.image.width == 4 && result.image.height == 4 &&
         matchesPixel(result.image, 0, 255, 0, 0, 255) &&
         matchesPixel(result.image, 1, 0, 255, 0, 255) &&
         matchesPixel(result.image, 2, 170, 85, 0, 255) &&
         matchesPixel(result.image, 3, 85, 170, 0, 255);
}

bool testBc3Alpha() {
  std::vector<std::byte> bytes = ddsHeader(fourCc('D', 'X', 'T', '5'), 16);
  bytes[128] = std::byte{10};
  bytes[129] = std::byte{20};
  std::uint64_t alphaIndices = 0;
  for (std::size_t pixel = 0; pixel < 16; ++pixel) {
    alphaIndices |= static_cast<std::uint64_t>(pixel % 8U) << (pixel * 3U);
  }
  for (std::size_t index = 0; index < 6; ++index) {
    bytes[130U + index] = static_cast<std::byte>((alphaIndices >> (index * 8U)) & 0xffU);
  }
  writeU16(bytes, 136, 0xf800);
  writeU16(bytes, 138, 0x07e0);
  writeU32(bytes, 140, 0);
  const tmnf::image::DecodeResult result = tmnf::image::decodeDds(bytes);
  constexpr std::uint8_t expectedAlpha[]{10, 20, 12, 14, 16, 18, 0, 255};
  if (!result) {
    return false;
  }
  for (std::size_t pixel = 0; pixel < 16; ++pixel) {
    if (!matchesPixel(result.image, pixel, 255, 0, 0, expectedAlpha[pixel % 8U])) {
      return false;
    }
  }
  return true;
}

bool testRawBgra() {
  std::vector<std::byte> bytes(132);
  bytes[0] = std::byte{'D'};
  bytes[1] = std::byte{'D'};
  bytes[2] = std::byte{'S'};
  bytes[3] = std::byte{' '};
  writeU32(bytes, 4, 124);
  writeU32(bytes, 8, 0x0000100f);
  writeU32(bytes, 12, 1);
  writeU32(bytes, 16, 1);
  writeU32(bytes, 20, 4);
  writeU32(bytes, 76, 32);
  writeU32(bytes, 80, 0x41);
  writeU32(bytes, 88, 32);
  writeU32(bytes, 92, 0x00ff0000);
  writeU32(bytes, 96, 0x0000ff00);
  writeU32(bytes, 100, 0x000000ff);
  writeU32(bytes, 104, 0xff000000);
  bytes[128] = std::byte{30};
  bytes[129] = std::byte{20};
  bytes[130] = std::byte{10};
  bytes[131] = std::byte{40};
  const tmnf::image::DecodeResult result = tmnf::image::decodeDds(bytes);
  return result && matchesPixel(result.image, 0, 10, 20, 30, 40);
}

bool testTgaOrientation() {
  std::vector<std::byte> bytes(30);
  bytes[2] = std::byte{2};
  writeU16(bytes, 12, 2);
  writeU16(bytes, 14, 2);
  bytes[16] = std::byte{24};
  constexpr std::uint8_t pixels[]{255, 0, 0, 255, 255, 255, 0, 0, 255, 0, 255, 0};
  for (std::size_t index = 0; index < std::size(pixels); ++index) {
    bytes[18U + index] = static_cast<std::byte>(pixels[index]);
  }
  const tmnf::image::DecodeResult result = tmnf::image::decodeTga(bytes);
  return result && matchesPixel(result.image, 0, 255, 0, 0, 255) &&
         matchesPixel(result.image, 1, 0, 255, 0, 255) &&
         matchesPixel(result.image, 2, 0, 0, 255, 255) &&
         matchesPixel(result.image, 3, 255, 255, 255, 255);
}

bool testTgaRle() {
  std::vector<std::byte> bytes(20);
  bytes[2] = std::byte{11};
  writeU16(bytes, 12, 3);
  writeU16(bytes, 14, 1);
  bytes[16] = std::byte{8};
  bytes[17] = std::byte{0x20};
  bytes[18] = std::byte{0x82};
  bytes[19] = std::byte{127};
  const tmnf::image::DecodeResult result = tmnf::image::decodeTga(bytes);
  return result && matchesPixel(result.image, 0, 127, 127, 127, 255) &&
         matchesPixel(result.image, 2, 127, 127, 127, 255);
}

} // namespace

int main() {
  if (!testBc1() || !testBc3Alpha() || !testRawBgra() || !testTgaOrientation() || !testTgaRle()) {
    std::cerr << "portable image decoding failed\n";
    return 1;
  }
  return 0;
}
