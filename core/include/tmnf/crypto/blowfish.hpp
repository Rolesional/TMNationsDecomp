#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace tmnf::crypto {

// Blowfish with the little-endian key words and blocks used by NadeoPak.
class BlowfishLittleEndian final {
public:
  explicit BlowfishLittleEndian(std::span<const std::byte> key);

  void encryptBlock(std::span<std::byte, 8> block) const noexcept;
  void decryptBlock(std::span<std::byte, 8> block) const noexcept;

private:
  [[nodiscard]] std::uint32_t substitute(std::uint32_t value) const noexcept;
  void encrypt(std::uint32_t& left, std::uint32_t& right) const noexcept;
  void decrypt(std::uint32_t& left, std::uint32_t& right) const noexcept;

  std::array<std::uint32_t, 18> p_{};
  std::array<std::uint32_t, 1024> s_{};
};

} // namespace tmnf::crypto
