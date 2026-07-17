#include "tmnf/image/dds.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>

namespace tmnf::image {
namespace {

constexpr std::size_t kHeaderSize = 128;
constexpr std::uint32_t kDdsdPitch = 0x00000008U;
constexpr std::uint32_t kDdpfAlphaPixels = 0x00000001U;
constexpr std::uint32_t kDdpfAlpha = 0x00000002U;
constexpr std::uint32_t kDdpfFourCc = 0x00000004U;
constexpr std::uint32_t kDdpfRgb = 0x00000040U;
constexpr std::uint32_t kDdpfLuminance = 0x00020000U;
constexpr std::uint32_t kMaximumDimension = 32'768U;

struct Color final {
  std::uint8_t red = 0;
  std::uint8_t green = 0;
  std::uint8_t blue = 0;
  std::uint8_t alpha = 255;
};

constexpr std::uint32_t fourCc(const char a, const char b, const char c, const char d) noexcept {
  return static_cast<std::uint32_t>(a) | (static_cast<std::uint32_t>(b) << 8U) |
         (static_cast<std::uint32_t>(c) << 16U) | (static_cast<std::uint32_t>(d) << 24U);
}

std::uint16_t readU16(const std::byte* const bytes) noexcept {
  return static_cast<std::uint16_t>(std::to_integer<unsigned char>(bytes[0])) |
         static_cast<std::uint16_t>(std::to_integer<unsigned char>(bytes[1]) << 8U);
}

std::uint32_t readU32(const std::byte* const bytes) noexcept {
  std::uint32_t value = 0;
  for (std::size_t index = 0; index < 4; ++index) {
    value |= static_cast<std::uint32_t>(std::to_integer<unsigned char>(bytes[index]))
             << (index * 8U);
  }
  return value;
}

std::uint64_t readU64(const std::byte* const bytes) noexcept {
  std::uint64_t value = 0;
  for (std::size_t index = 0; index < 8; ++index) {
    value |= static_cast<std::uint64_t>(std::to_integer<unsigned char>(bytes[index]))
             << (index * 8U);
  }
  return value;
}

DecodeResult failed(std::string message) {
  DecodeResult result;
  result.error = std::move(message);
  return result;
}

bool prepareImage(const std::uint32_t width, const std::uint32_t height, DecodeResult& result) {
  if (width == 0 || height == 0 || width > kMaximumDimension || height > kMaximumDimension ||
      width > std::numeric_limits<std::size_t>::max() / height) {
    return false;
  }
  const std::size_t pixelCount = static_cast<std::size_t>(width) * height;
  if (pixelCount > std::numeric_limits<std::size_t>::max() / 4U ||
      width > static_cast<std::uint32_t>(std::numeric_limits<int>::max()) ||
      height > static_cast<std::uint32_t>(std::numeric_limits<int>::max())) {
    return false;
  }
  result.image.width = static_cast<int>(width);
  result.image.height = static_cast<int>(height);
  result.image.pixels.resize(pixelCount * 4U);
  return true;
}

std::uint8_t expand5(const std::uint16_t value) noexcept {
  return static_cast<std::uint8_t>((value << 3U) | (value >> 2U));
}

std::uint8_t expand6(const std::uint16_t value) noexcept {
  return static_cast<std::uint8_t>((value << 2U) | (value >> 4U));
}

Color decode565(const std::uint16_t value) noexcept {
  return Color{expand5(static_cast<std::uint16_t>((value >> 11U) & 31U)),
               expand6(static_cast<std::uint16_t>((value >> 5U) & 63U)),
               expand5(static_cast<std::uint16_t>(value & 31U)), 255};
}

std::array<Color, 4> colorPalette(const std::byte* const block,
                                  const bool allowTransparency) noexcept {
  const std::uint16_t encoded0 = readU16(block);
  const std::uint16_t encoded1 = readU16(block + 2);
  std::array<Color, 4> colors{};
  colors[0] = decode565(encoded0);
  colors[1] = decode565(encoded1);
  if (!allowTransparency || encoded0 > encoded1) {
    colors[2] = Color{static_cast<std::uint8_t>((2U * colors[0].red + colors[1].red) / 3U),
                      static_cast<std::uint8_t>((2U * colors[0].green + colors[1].green) / 3U),
                      static_cast<std::uint8_t>((2U * colors[0].blue + colors[1].blue) / 3U), 255};
    colors[3] = Color{static_cast<std::uint8_t>((colors[0].red + 2U * colors[1].red) / 3U),
                      static_cast<std::uint8_t>((colors[0].green + 2U * colors[1].green) / 3U),
                      static_cast<std::uint8_t>((colors[0].blue + 2U * colors[1].blue) / 3U), 255};
  } else {
    colors[2] = Color{static_cast<std::uint8_t>((colors[0].red + colors[1].red) / 2U),
                      static_cast<std::uint8_t>((colors[0].green + colors[1].green) / 2U),
                      static_cast<std::uint8_t>((colors[0].blue + colors[1].blue) / 2U), 255};
    colors[3] = Color{0, 0, 0, 0};
  }
  return colors;
}

void storePixel(RgbaImage& image, const std::uint32_t x, const std::uint32_t y, const Color color) {
  const std::size_t offset = (static_cast<std::size_t>(y) * image.width + x) * 4U;
  image.pixels[offset] = color.red;
  image.pixels[offset + 1U] = color.green;
  image.pixels[offset + 2U] = color.blue;
  image.pixels[offset + 3U] = color.alpha;
}

void decodeColorIndices(RgbaImage& image, const std::uint32_t blockX, const std::uint32_t blockY,
                        const std::byte* const encodedColors, const bool allowTransparency,
                        const std::array<std::uint8_t, 16>& alpha) {
  const std::array<Color, 4> colors = colorPalette(encodedColors, allowTransparency);
  const std::uint32_t indices = readU32(encodedColors + 4);
  for (std::uint32_t y = 0; y < 4; ++y) {
    for (std::uint32_t x = 0; x < 4; ++x) {
      const std::uint32_t destinationX = blockX * 4U + x;
      const std::uint32_t destinationY = blockY * 4U + y;
      if (destinationX >= static_cast<std::uint32_t>(image.width) ||
          destinationY >= static_cast<std::uint32_t>(image.height)) {
        continue;
      }
      const std::size_t pixel = y * 4U + x;
      Color color = colors[(indices >> (pixel * 2U)) & 3U];
      color.alpha =
          static_cast<std::uint8_t>((static_cast<unsigned int>(color.alpha) * alpha[pixel]) / 255U);
      storePixel(image, destinationX, destinationY, color);
    }
  }
}

DecodeResult decodeBlocks(const std::span<const std::byte> bytes, const std::uint32_t width,
                          const std::uint32_t height, const std::uint32_t encodedFormat) {
  enum class BlockFormat { Bc1, Bc2, Bc3 };
  BlockFormat format = BlockFormat::Bc1;
  std::size_t blockSize = 8;
  std::string formatName = "BC1/DXT1";
  if (encodedFormat == fourCc('D', 'X', 'T', '3')) {
    format = BlockFormat::Bc2;
    blockSize = 16;
    formatName = "BC2/DXT3";
  } else if (encodedFormat == fourCc('D', 'X', 'T', '5')) {
    format = BlockFormat::Bc3;
    blockSize = 16;
    formatName = "BC3/DXT5";
  } else if (encodedFormat != fourCc('D', 'X', 'T', '1')) {
    if (encodedFormat == fourCc('D', 'X', '1', '0')) {
      return failed("DDS DX10 headers are not supported");
    }
    return failed("unsupported DDS FourCC");
  }

  const std::size_t blockWidth = (static_cast<std::size_t>(width) + 3U) / 4U;
  const std::size_t blockHeight = (static_cast<std::size_t>(height) + 3U) / 4U;
  if (blockWidth > std::numeric_limits<std::size_t>::max() / blockHeight ||
      blockWidth * blockHeight > (bytes.size() - kHeaderSize) / blockSize) {
    return failed("DDS block data is truncated");
  }

  DecodeResult result;
  result.format = std::move(formatName);
  if (!prepareImage(width, height, result)) {
    return failed("invalid DDS dimensions");
  }
  std::size_t offset = kHeaderSize;
  for (std::uint32_t blockY = 0; blockY < blockHeight; ++blockY) {
    for (std::uint32_t blockX = 0; blockX < blockWidth; ++blockX) {
      const std::byte* const block = bytes.data() + offset;
      std::array<std::uint8_t, 16> alpha{};
      alpha.fill(255);
      const std::byte* colors = block;
      bool allowTransparency = true;
      if (format == BlockFormat::Bc2) {
        const std::uint64_t alphaBits = readU64(block);
        for (std::size_t pixel = 0; pixel < alpha.size(); ++pixel) {
          alpha[pixel] = static_cast<std::uint8_t>(((alphaBits >> (pixel * 4U)) & 0x0fU) * 17U);
        }
        colors += 8;
        allowTransparency = false;
      } else if (format == BlockFormat::Bc3) {
        std::array<std::uint8_t, 8> palette{};
        palette[0] = std::to_integer<std::uint8_t>(block[0]);
        palette[1] = std::to_integer<std::uint8_t>(block[1]);
        if (palette[0] > palette[1]) {
          for (std::size_t index = 1; index <= 6; ++index) {
            palette[index + 1U] =
                static_cast<std::uint8_t>(((7U - index) * palette[0] + index * palette[1]) / 7U);
          }
        } else {
          for (std::size_t index = 1; index <= 4; ++index) {
            palette[index + 1U] =
                static_cast<std::uint8_t>(((5U - index) * palette[0] + index * palette[1]) / 5U);
          }
          palette[6] = 0;
          palette[7] = 255;
        }
        std::uint64_t alphaBits = 0;
        for (std::size_t index = 0; index < 6; ++index) {
          alphaBits |= static_cast<std::uint64_t>(std::to_integer<unsigned char>(block[index + 2U]))
                       << (index * 8U);
        }
        for (std::size_t pixel = 0; pixel < alpha.size(); ++pixel) {
          alpha[pixel] = palette[(alphaBits >> (pixel * 3U)) & 7U];
        }
        colors += 8;
        allowTransparency = false;
      }
      decodeColorIndices(result.image, blockX, blockY, colors, allowTransparency, alpha);
      offset += blockSize;
    }
  }
  return result;
}

std::uint8_t maskedChannel(const std::uint32_t pixel, const std::uint32_t mask,
                           const std::uint8_t fallback) noexcept {
  if (mask == 0) {
    return fallback;
  }
  const unsigned int shift = std::countr_zero(mask);
  const std::uint32_t maximum = mask >> shift;
  const std::uint32_t value = (pixel & mask) >> shift;
  return static_cast<std::uint8_t>((static_cast<std::uint64_t>(value) * 255U + maximum / 2U) /
                                   maximum);
}

DecodeResult decodeRaw(const std::span<const std::byte> bytes, const std::uint32_t width,
                       const std::uint32_t height, const std::uint32_t headerFlags,
                       const std::uint32_t pixelFlags) {
  const std::uint32_t bitCount = readU32(bytes.data() + 88);
  if (bitCount != 8 && bitCount != 16 && bitCount != 24 && bitCount != 32) {
    return failed("unsupported DDS RGB bit count");
  }
  const std::size_t bytesPerPixel = bitCount / 8U;
  if (width > std::numeric_limits<std::size_t>::max() / bytesPerPixel) {
    return failed("invalid DDS row size");
  }
  const std::size_t rowBytes = static_cast<std::size_t>(width) * bytesPerPixel;
  const std::uint32_t encodedPitch = readU32(bytes.data() + 20);
  const std::size_t pitch =
      (headerFlags & kDdsdPitch) != 0 && encodedPitch >= rowBytes ? encodedPitch : rowBytes;
  if (height > 0 && (height - 1U) > (bytes.size() - kHeaderSize) / pitch) {
    return failed("DDS pixel data is truncated");
  }
  const std::size_t finalRowOffset = static_cast<std::size_t>(height - 1U) * pitch;
  if (finalRowOffset > bytes.size() - kHeaderSize ||
      rowBytes > bytes.size() - kHeaderSize - finalRowOffset) {
    return failed("DDS pixel data is truncated");
  }

  DecodeResult result;
  result.format = (pixelFlags & kDdpfRgb) != 0         ? "RGB"
                  : (pixelFlags & kDdpfLuminance) != 0 ? "luminance"
                                                       : "alpha";
  if (!prepareImage(width, height, result)) {
    return failed("invalid DDS dimensions");
  }
  const std::uint32_t redMask = readU32(bytes.data() + 92);
  const std::uint32_t greenMask = readU32(bytes.data() + 96);
  const std::uint32_t blueMask = readU32(bytes.data() + 100);
  const std::uint32_t alphaMask = readU32(bytes.data() + 104);
  if ((pixelFlags & kDdpfRgb) != 0 && (redMask == 0 || greenMask == 0 || blueMask == 0)) {
    return failed("DDS RGB masks are missing");
  }

  for (std::uint32_t y = 0; y < height; ++y) {
    const std::byte* row = bytes.data() + kHeaderSize + static_cast<std::size_t>(y) * pitch;
    for (std::uint32_t x = 0; x < width; ++x) {
      std::uint32_t pixel = 0;
      for (std::size_t byteIndex = 0; byteIndex < bytesPerPixel; ++byteIndex) {
        pixel |= static_cast<std::uint32_t>(
                     std::to_integer<unsigned char>(row[x * bytesPerPixel + byteIndex]))
                 << (byteIndex * 8U);
      }
      Color color;
      if ((pixelFlags & kDdpfRgb) != 0) {
        color.red = maskedChannel(pixel, redMask, 0);
        color.green = maskedChannel(pixel, greenMask, 0);
        color.blue = maskedChannel(pixel, blueMask, 0);
        color.alpha = maskedChannel(pixel, alphaMask, 255);
      } else if ((pixelFlags & kDdpfLuminance) != 0) {
        const std::uint32_t defaultMask =
            bitCount == 32 ? std::numeric_limits<std::uint32_t>::max() : (1U << bitCount) - 1U;
        const std::uint32_t luminanceMask = redMask != 0 ? redMask : defaultMask;
        color.red = maskedChannel(pixel, luminanceMask, 0);
        color.green = color.red;
        color.blue = color.red;
        color.alpha = maskedChannel(pixel, alphaMask, 255);
      } else if ((pixelFlags & kDdpfAlpha) != 0) {
        color = Color{255, 255, 255, maskedChannel(pixel, alphaMask != 0 ? alphaMask : 0xffU, 255)};
      } else {
        return failed("unsupported DDS pixel flags");
      }
      storePixel(result.image, x, y, color);
    }
  }
  return result;
}

} // namespace

DecodeResult decodeDds(const std::span<const std::byte> bytes) {
  if (bytes.size() < kHeaderSize) {
    return failed("DDS header is truncated");
  }
  if (std::to_integer<char>(bytes[0]) != 'D' || std::to_integer<char>(bytes[1]) != 'D' ||
      std::to_integer<char>(bytes[2]) != 'S' || std::to_integer<char>(bytes[3]) != ' ' ||
      readU32(bytes.data() + 4) != 124U || readU32(bytes.data() + 76) != 32U) {
    return failed("invalid DDS header");
  }
  const std::uint32_t height = readU32(bytes.data() + 12);
  const std::uint32_t width = readU32(bytes.data() + 16);
  if (width == 0 || height == 0 || width > kMaximumDimension || height > kMaximumDimension) {
    return failed("invalid DDS dimensions");
  }
  const std::uint32_t headerFlags = readU32(bytes.data() + 8);
  const std::uint32_t pixelFlags = readU32(bytes.data() + 80);
  if ((pixelFlags & kDdpfFourCc) != 0) {
    return decodeBlocks(bytes, width, height, readU32(bytes.data() + 84));
  }
  if ((pixelFlags & (kDdpfRgb | kDdpfLuminance | kDdpfAlpha | kDdpfAlphaPixels)) != 0) {
    return decodeRaw(bytes, width, height, headerFlags, pixelFlags);
  }
  return failed("unsupported DDS pixel format");
}

} // namespace tmnf::image
