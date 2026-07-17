#include "tmnf/image/tga.hpp"

#include <array>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>

namespace tmnf::image {
namespace {

constexpr std::size_t kHeaderSize = 18;
constexpr std::uint16_t kMaximumDimension = 32'768U;

std::uint16_t readU16(const std::byte* const bytes) noexcept {
  return static_cast<std::uint16_t>(std::to_integer<unsigned char>(bytes[0])) |
         static_cast<std::uint16_t>(std::to_integer<unsigned char>(bytes[1]) << 8U);
}

DecodeResult failed(std::string message) {
  DecodeResult result;
  result.error = std::move(message);
  return result;
}

} // namespace

DecodeResult decodeTga(const std::span<const std::byte> bytes) {
  if (bytes.size() < kHeaderSize) {
    return failed("TGA header is truncated");
  }
  const std::size_t idLength = std::to_integer<unsigned char>(bytes[0]);
  const unsigned int colorMapType = std::to_integer<unsigned char>(bytes[1]);
  const unsigned int imageType = std::to_integer<unsigned char>(bytes[2]);
  const std::uint16_t width = readU16(bytes.data() + 12);
  const std::uint16_t height = readU16(bytes.data() + 14);
  const unsigned int bitsPerPixel = std::to_integer<unsigned char>(bytes[16]);
  const unsigned int descriptor = std::to_integer<unsigned char>(bytes[17]);
  const bool trueColor = imageType == 2 || imageType == 10;
  const bool grayscale = imageType == 3 || imageType == 11;
  const bool rle = imageType == 10 || imageType == 11;
  if (colorMapType != 0 || (!trueColor && !grayscale)) {
    return failed("unsupported TGA image type");
  }
  if ((trueColor && bitsPerPixel != 16 && bitsPerPixel != 24 && bitsPerPixel != 32) ||
      (grayscale && bitsPerPixel != 8 && bitsPerPixel != 16)) {
    return failed("unsupported TGA pixel layout");
  }
  if (width == 0 || height == 0 || width > kMaximumDimension || height > kMaximumDimension) {
    return failed("invalid TGA dimensions");
  }
  const std::size_t pixelCount = static_cast<std::size_t>(width) * height;
  if (pixelCount > std::numeric_limits<std::size_t>::max() / 4U ||
      idLength > bytes.size() - kHeaderSize) {
    return failed("invalid TGA size");
  }
  const std::size_t bytesPerPixel = bitsPerPixel / 8U;
  std::size_t cursor = kHeaderSize + idLength;

  DecodeResult result;
  result.format = rle ? "TGA RLE" : "TGA";
  result.image.width = width;
  result.image.height = height;
  result.image.pixels.resize(pixelCount * 4U);

  const bool topOrigin = (descriptor & 0x20U) != 0;
  const bool rightOrigin = (descriptor & 0x10U) != 0;
  auto store = [&](const std::size_t sequentialIndex, const std::byte* const source) {
    const std::size_t sourceX = sequentialIndex % width;
    const std::size_t sourceY = sequentialIndex / width;
    const std::size_t x = rightOrigin ? width - sourceX - 1U : sourceX;
    const std::size_t y = topOrigin ? sourceY : height - sourceY - 1U;
    const std::size_t destination = (y * width + x) * 4U;
    std::array<std::uint8_t, 4> color{0, 0, 0, 255};
    if (grayscale) {
      color[0] = std::to_integer<std::uint8_t>(source[0]);
      color[1] = color[0];
      color[2] = color[0];
      if (bitsPerPixel == 16) {
        color[3] = std::to_integer<std::uint8_t>(source[1]);
      }
    } else if (bitsPerPixel == 16) {
      const std::uint16_t packed = readU16(source);
      color[0] = static_cast<std::uint8_t>((((packed >> 10U) & 31U) * 255U + 15U) / 31U);
      color[1] = static_cast<std::uint8_t>((((packed >> 5U) & 31U) * 255U + 15U) / 31U);
      color[2] = static_cast<std::uint8_t>(((packed & 31U) * 255U + 15U) / 31U);
      if ((descriptor & 0x0fU) != 0) {
        color[3] = (packed & 0x8000U) != 0 ? 255 : 0;
      }
    } else {
      color[0] = std::to_integer<std::uint8_t>(source[2]);
      color[1] = std::to_integer<std::uint8_t>(source[1]);
      color[2] = std::to_integer<std::uint8_t>(source[0]);
      if (bitsPerPixel == 32) {
        color[3] = std::to_integer<std::uint8_t>(source[3]);
      }
    }
    for (std::size_t channel = 0; channel < color.size(); ++channel) {
      result.image.pixels[destination + channel] = color[channel];
    }
  };

  std::size_t decodedPixels = 0;
  while (decodedPixels < pixelCount) {
    std::size_t packetPixels = pixelCount - decodedPixels;
    bool repeated = false;
    if (rle) {
      if (cursor >= bytes.size()) {
        return failed("TGA RLE packet is truncated");
      }
      const unsigned int packet = std::to_integer<unsigned char>(bytes[cursor++]);
      packetPixels = (packet & 0x7fU) + 1U;
      repeated = (packet & 0x80U) != 0;
      if (packetPixels > pixelCount - decodedPixels) {
        return failed("TGA RLE packet exceeds the image");
      }
    }
    const std::size_t sourcePixels = repeated ? 1U : packetPixels;
    if (sourcePixels > (bytes.size() - cursor) / bytesPerPixel) {
      return failed("TGA pixel data is truncated");
    }
    if (repeated) {
      for (std::size_t index = 0; index < packetPixels; ++index) {
        store(decodedPixels++, bytes.data() + cursor);
      }
      cursor += bytesPerPixel;
    } else {
      for (std::size_t index = 0; index < packetPixels; ++index) {
        store(decodedPixels++, bytes.data() + cursor);
        cursor += bytesPerPixel;
      }
    }
  }
  return result;
}

} // namespace tmnf::image
