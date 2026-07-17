#include "tmnf/pak/cipher.hpp"

#include <algorithm>
#include <array>
#include <bit>

namespace tmnf::pak {
namespace {

std::uint64_t readLeU64(const std::byte* const bytes) noexcept {
  std::uint64_t value = 0;
  for (std::size_t index = 0; index < 8; ++index) {
    value |= static_cast<std::uint64_t>(std::to_integer<unsigned char>(bytes[index]))
             << (index * 8U);
  }
  return value;
}

void writeLeU64(const std::uint64_t value, std::byte* const bytes) noexcept {
  for (std::size_t index = 0; index < 8; ++index) {
    bytes[index] = static_cast<std::byte>((value >> (index * 8U)) & 0xffU);
  }
}

} // namespace

CipherReader::CipherReader(const std::span<const std::byte> ciphertext,
                           const crypto::Md5Digest& key, const std::uint64_t initializationVector)
    : ciphertext_(ciphertext), blowfish_(key), initializationVector_(initializationVector) {}

bool CipherReader::readBytes(const std::span<std::byte> destination) {
  for (std::byte& value : destination) {
    if (!readByte(value)) {
      return false;
    }
  }
  return true;
}

bool CipherReader::readU32(std::uint32_t& value) {
  std::array<std::byte, 4> bytes{};
  if (!readBytes(bytes)) {
    return false;
  }
  value = 0;
  for (std::size_t index = 0; index < bytes.size(); ++index) {
    value |= static_cast<std::uint32_t>(std::to_integer<unsigned char>(bytes[index]))
             << (index * 8U);
  }
  return true;
}

bool CipherReader::readI32(std::int32_t& value) {
  std::uint32_t unsignedValue = 0;
  if (!readU32(unsignedValue)) {
    return false;
  }
  value = std::bit_cast<std::int32_t>(unsignedValue);
  return true;
}

bool CipherReader::readU64(std::uint64_t& value) {
  std::array<std::byte, 8> bytes{};
  if (!readBytes(bytes)) {
    return false;
  }
  value = readLeU64(bytes.data());
  return true;
}

bool CipherReader::readString(std::string& value) {
  std::int32_t length = 0;
  if (!readI32(length) || length < 0 || length > (1 << 28)) {
    return false;
  }
  value.resize(static_cast<std::size_t>(length));
  return readBytes(std::span(reinterpret_cast<std::byte*>(value.data()), value.size()));
}

void CipherReader::initialize(const std::span<const std::byte> bytes) noexcept {
  for (const std::byte value : bytes) {
    initializationXor_ = std::rotl(initializationXor_, 13);
    initializationXor_ ^= static_cast<std::uint64_t>(std::to_integer<unsigned char>(value) | 0xaaU);
  }
}

bool CipherReader::readByte(std::byte& value) {
  if (cycleIndex_ % decryptedBlock_.size() == 0) {
    if (plainPosition_ == 0) {
      initializationVector_ ^= initializationXor_;
      initializationXor_ = 0;
    }
    if (cycleIndex_ == 0x100U) {
      initializationVector_ ^= initializationXor_;
      initializationXor_ = 0;
      cycleIndex_ = 0;
    }
    if (cipherPosition_ > ciphertext_.size() ||
        ciphertext_.size() - cipherPosition_ < decryptedBlock_.size()) {
      return false;
    }
    std::copy_n(ciphertext_.begin() + static_cast<std::ptrdiff_t>(cipherPosition_),
                decryptedBlock_.size(), decryptedBlock_.begin());
    cipherPosition_ += decryptedBlock_.size();
    const std::uint64_t nextInitializationVector = readLeU64(decryptedBlock_.data());
    blowfish_.decryptBlock(decryptedBlock_);
    const std::uint64_t plainBlock = readLeU64(decryptedBlock_.data()) ^ initializationVector_;
    writeLeU64(plainBlock, decryptedBlock_.data());
    initializationVector_ = nextInitializationVector;
  }

  value = decryptedBlock_[cycleIndex_ & 7U];
  ++cycleIndex_;
  ++plainPosition_;
  return true;
}

} // namespace tmnf::pak
