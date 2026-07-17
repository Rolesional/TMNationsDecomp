#pragma once

#include "tmnf/crypto/blowfish.hpp"
#include "tmnf/crypto/md5.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>

namespace tmnf::pak {

// Streaming decoder for the version-3 NadeoPak Blowfish/CBC variant. The
// initialize operation reproduces CClassicBufferCrypted's dummy-write reseed.
class CipherReader final {
public:
  CipherReader(std::span<const std::byte> ciphertext, const crypto::Md5Digest& key,
               std::uint64_t initializationVector);

  bool readBytes(std::span<std::byte> destination);
  bool readU32(std::uint32_t& value);
  bool readI32(std::int32_t& value);
  bool readU64(std::uint64_t& value);
  bool readString(std::string& value);
  void initialize(std::span<const std::byte> bytes) noexcept;

  [[nodiscard]] std::size_t position() const noexcept { return plainPosition_; }
  [[nodiscard]] std::size_t cycleBytesRemaining() const noexcept {
    return cycleIndex_ == 0x100U ? 0x100U : 0x100U - cycleIndex_;
  }

private:
  bool readByte(std::byte& value);

  std::span<const std::byte> ciphertext_;
  crypto::BlowfishLittleEndian blowfish_;
  std::uint64_t initializationVector_ = 0;
  std::uint64_t initializationXor_ = 0;
  std::array<std::byte, 8> decryptedBlock_{};
  std::size_t cipherPosition_ = 0;
  std::size_t plainPosition_ = 0;
  std::size_t cycleIndex_ = 0;
};

} // namespace tmnf::pak
