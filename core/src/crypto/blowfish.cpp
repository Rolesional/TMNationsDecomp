#include "tmnf/crypto/blowfish.hpp"

#include <stdexcept>

namespace tmnf::crypto {
namespace {

#include "blowfish_constants.inc"

std::uint32_t readU32(const std::byte* const bytes) noexcept {
  std::uint32_t value = 0;
  for (std::size_t index = 0; index < 4; ++index) {
    value |= static_cast<std::uint32_t>(std::to_integer<unsigned char>(bytes[index]))
             << (index * 8U);
  }
  return value;
}

void writeU32(const std::uint32_t value, std::byte* const bytes) noexcept {
  for (std::size_t index = 0; index < 4; ++index) {
    bytes[index] = static_cast<std::byte>((value >> (index * 8U)) & 0xffU);
  }
}

} // namespace

BlowfishLittleEndian::BlowfishLittleEndian(const std::span<const std::byte> key)
    : p_(kInitialP), s_(kInitialS) {
  if (key.empty()) {
    throw std::invalid_argument("Blowfish key cannot be empty");
  }

  std::size_t keyIndex = 0;
  for (std::uint32_t& value : p_) {
    std::uint32_t keyWord = 0;
    for (std::size_t byteIndex = 0; byteIndex < 4; ++byteIndex) {
      keyWord |= static_cast<std::uint32_t>(std::to_integer<unsigned char>(key[keyIndex]))
                 << (byteIndex * 8U);
      keyIndex = (keyIndex + 1U) % key.size();
    }
    value ^= keyWord;
  }

  std::uint32_t left = 0;
  std::uint32_t right = 0;
  for (std::size_t index = 0; index < p_.size(); index += 2) {
    encrypt(left, right);
    p_[index] = left;
    p_[index + 1U] = right;
  }
  for (std::size_t index = 0; index < s_.size(); index += 2) {
    encrypt(left, right);
    s_[index] = left;
    s_[index + 1U] = right;
  }
}

std::uint32_t BlowfishLittleEndian::substitute(const std::uint32_t value) const noexcept {
  const std::size_t a = (value >> 24U) & 0xffU;
  const std::size_t b = (value >> 16U) & 0xffU;
  const std::size_t c = (value >> 8U) & 0xffU;
  const std::size_t d = value & 0xffU;
  return ((s_[a] + s_[256U + b]) ^ s_[512U + c]) + s_[768U + d];
}

void BlowfishLittleEndian::encrypt(std::uint32_t& left, std::uint32_t& right) const noexcept {
  for (std::size_t index = 0; index < 16; ++index) {
    left ^= p_[index];
    right ^= substitute(left);
    const std::uint32_t temporary = left;
    left = right;
    right = temporary;
  }
  const std::uint32_t temporary = left;
  left = right;
  right = temporary;
  right ^= p_[16];
  left ^= p_[17];
}

void BlowfishLittleEndian::decrypt(std::uint32_t& left, std::uint32_t& right) const noexcept {
  for (std::size_t index = 17; index > 1; --index) {
    left ^= p_[index];
    right ^= substitute(left);
    const std::uint32_t temporary = left;
    left = right;
    right = temporary;
  }
  const std::uint32_t temporary = left;
  left = right;
  right = temporary;
  left ^= p_[0];
  right ^= p_[1];
}

void BlowfishLittleEndian::encryptBlock(const std::span<std::byte, 8> block) const noexcept {
  std::uint32_t left = readU32(block.data());
  std::uint32_t right = readU32(block.data() + 4);
  encrypt(left, right);
  writeU32(left, block.data());
  writeU32(right, block.data() + 4);
}

void BlowfishLittleEndian::decryptBlock(const std::span<std::byte, 8> block) const noexcept {
  std::uint32_t left = readU32(block.data());
  std::uint32_t right = readU32(block.data() + 4);
  decrypt(left, right);
  writeU32(left, block.data());
  writeU32(right, block.data() + 4);
}

} // namespace tmnf::crypto
