#include "tmnf/compression/lzo.hpp"

#include <array>
#include <cstddef>
#include <iostream>
#include <span>
#include <string_view>

int main() {
  // One 12-byte literal run followed by the canonical LZO1X end marker.
  constexpr std::array<std::byte, 16> compressed{
      std::byte{29},  std::byte{'H'}, std::byte{'e'}, std::byte{'l'},
      std::byte{'l'}, std::byte{'o'}, std::byte{' '}, std::byte{'W'},
      std::byte{'o'}, std::byte{'r'}, std::byte{'l'}, std::byte{'d'},
      std::byte{'!'}, std::byte{17},  std::byte{0},   std::byte{0},
  };
  const tmnf::compression::LzoDecompressResult decoded =
      tmnf::compression::decompressLzo1x(compressed, 12);
  if (!decoded) {
    std::cerr << decoded.error << '\n';
    return 1;
  }
  const std::string_view text(reinterpret_cast<const char*>(decoded.data.data()),
                              decoded.data.size());
  if (text != "Hello World!") {
    std::cerr << "literal LZO fixture decoded incorrectly\n";
    return 1;
  }

  const tmnf::compression::LzoDecompressResult wrongSize =
      tmnf::compression::decompressLzo1x(compressed, 11);
  if (wrongSize) {
    std::cerr << "LZO output overrun was accepted\n";
    return 1;
  }
  const tmnf::compression::LzoDecompressResult truncated =
      tmnf::compression::decompressLzo1x(std::span(compressed).first(14), 12);
  if (truncated) {
    std::cerr << "truncated LZO end marker was accepted\n";
    return 1;
  }
  return 0;
}
