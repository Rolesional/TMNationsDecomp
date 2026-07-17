#pragma once

#include "tmnf/image/image.hpp"

#include <cstddef>
#include <span>

namespace tmnf::image {

// Decodes uncompressed or RLE true-color/grayscale TGA into top-left-origin RGBA8.
[[nodiscard]] DecodeResult decodeTga(std::span<const std::byte> bytes);

} // namespace tmnf::image
