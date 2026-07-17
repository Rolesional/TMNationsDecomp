#pragma once

#include "tmnf/crypto/md5.hpp"
#include "tmnf/gbx/input.hpp"
#include "tmnf/pak/archive.hpp"
#include "tmnf/pak/cipher.hpp"

#include <array>
#include <cstddef>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace tmnf::pak {

struct FileDataResult final {
  std::vector<std::byte> data;
  std::string error;

  [[nodiscard]] explicit operator bool() const noexcept { return error.empty(); }
};

// A pull reader for one archive entry. Compressed input is fed to zlib only up
// to the current 256-byte cipher-cycle boundary so parser-driven dummy writes
// can reseed the following cycle before it is decrypted.
class FileDataReader final : public gbx::Input {
public:
  FileDataReader(std::span<const std::byte> archiveBytes, const Archive& archive,
                 const FileEntry& file, const crypto::Md5Digest& archiveKey);
  ~FileDataReader() override;

  FileDataReader(const FileDataReader&) = delete;
  FileDataReader& operator=(const FileDataReader&) = delete;
  FileDataReader(FileDataReader&&) = delete;
  FileDataReader& operator=(FileDataReader&&) = delete;

  bool readBytes(std::span<std::byte> destination) override;
  void initialize(std::span<const std::byte> bytes) noexcept override;

  // Strictly consumes the archive-compression trailer after all declared
  // logical bytes have been read. Normal GBX parsing intentionally does not
  // do this: the original zlib wrapper stops at the requested output length,
  // and a late dummy-write seed may target ciphertext that contains only the
  // now-unused zlib checksum/trailer.
  bool finish();

  [[nodiscard]] explicit operator bool() const noexcept { return error_.empty(); }
  [[nodiscard]] std::string_view error() const noexcept override { return error_; }
  [[nodiscard]] std::size_t position() const noexcept override { return outputPosition_; }
  [[nodiscard]] std::size_t size() const noexcept override { return outputSize_; }

private:
  struct InflateState;

  bool readStored(std::span<std::byte> destination);
  bool refillInflateInput();
  bool refillInflateOutput();
  bool readInflated(std::span<std::byte> destination);
  bool validateInflateEnd();
  bool fail(std::string message);

  std::span<const std::byte> rawStored_;
  std::optional<CipherReader> cipher_;
  std::unique_ptr<InflateState> inflate_;
  std::array<std::byte, 0x100> inflateInput_{};
  std::array<std::byte, 0x400> inflateOutput_{};
  std::size_t storedPosition_ = 0;
  std::size_t storedSize_ = 0;
  std::size_t inflateOutputPosition_ = 0;
  std::size_t inflateOutputSize_ = 0;
  std::size_t inflateProduced_ = 0;
  std::size_t outputPosition_ = 0;
  std::size_t outputSize_ = 0;
  bool compressed_ = false;
  bool useDummyWrite_ = false;
  std::string error_;
};

// Decrypts and, when necessary, inflates one archive entry. GBX entries that
// cross a cipher boundary may require parser-driven dummy-write reseeding.
// This eager helper performs strict zlib-trailer validation because it never
// queues parser-driven seeds.
[[nodiscard]] FileDataResult readFileData(std::span<const std::byte> archiveBytes,
                                          const Archive& archive, const FileEntry& file,
                                          const crypto::Md5Digest& archiveKey);

// Returns the entry's decrypted stored representation before archive-level
// decompression, with a zero dummy-write accumulator. This is primarily an
// input to recovery and diagnostics; normal callers should use FileDataReader.
[[nodiscard]] FileDataResult
readStoredFileDataWithoutDummyWrites(std::span<const std::byte> archiveBytes,
                                     const Archive& archive, const FileEntry& file,
                                     const crypto::Md5Digest& archiveKey);

} // namespace tmnf::pak
