#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace tmnf::image {

struct RgbaImage final {
  int width = 0;
  int height = 0;
  std::vector<std::uint8_t> pixels;
};

struct DecodeResult final {
  RgbaImage image;
  std::string format;
  std::string error;

  [[nodiscard]] explicit operator bool() const noexcept { return error.empty(); }
};

} // namespace tmnf::image
