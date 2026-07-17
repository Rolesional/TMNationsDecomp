#pragma once

#include "tmnf/image/image.hpp"

#include <cstddef>
#include <span>

namespace tmnf::image {

// Decodes the top mip of legacy DDS textures into top-left-origin RGBA8.
// BC1/DXT1, BC2/DXT3, BC3/DXT5, RGB(A), luminance, and alpha layouts are supported.
[[nodiscard]] DecodeResult decodeDds(std::span<const std::byte> bytes);

} // namespace tmnf::image
