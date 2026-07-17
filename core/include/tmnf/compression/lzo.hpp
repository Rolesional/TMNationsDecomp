#pragma once

#include <cstddef>
#include <span>
#include <string>
#include <vector>

namespace tmnf::compression {

struct LzoDecompressResult final {
  std::vector<std::byte> data;
  std::string error;

  [[nodiscard]] explicit operator bool() const noexcept { return error.empty(); }
};

// Decompresses one raw LZO1X block into exactly expectedSize bytes.
[[nodiscard]] LzoDecompressResult decompressLzo1x(std::span<const std::byte> compressed,
                                                  std::size_t expectedSize);

} // namespace tmnf::compression
