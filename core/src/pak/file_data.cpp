#include "tmnf/pak/file_data.hpp"

#include <algorithm>
#include <cstdint>
#include <utility>

extern "C" {

using ZByte = unsigned char;
using ZUInt = unsigned int;
using ZULong = unsigned long;
using ZVoid = void*;
using ZAlloc = ZVoid (*)(ZVoid opaque, ZUInt items, ZUInt size);
using ZFree = void (*)(ZVoid opaque, ZVoid address);

struct ZInternalState;
struct ZStream {
  ZByte* nextIn;
  ZUInt availableIn;
  ZULong totalIn;
  ZByte* nextOut;
  ZUInt availableOut;
  ZULong totalOut;
  char* message;
  ZInternalState* state;
  ZAlloc allocate;
  ZFree free;
  ZVoid opaque;
  int dataType;
  ZULong adler;
  ZULong reserved;
};

const char* zlibVersion();
int inflateInit_(ZStream* stream, const char* version, int streamSize);
int inflate(ZStream* stream, int flush);
int inflateEnd(ZStream* stream);

} // extern "C"

namespace tmnf::pak {
namespace {

constexpr int kZlibNoFlush = 0;
constexpr int kZlibOk = 0;
constexpr int kZlibStreamEnd = 1;

std::uint64_t readLeU64(const std::byte* const bytes) noexcept {
  std::uint64_t value = 0;
  for (std::size_t index = 0; index < 8; ++index) {
    value |= static_cast<std::uint64_t>(std::to_integer<unsigned char>(bytes[index]))
             << (index * 8U);
  }
  return value;
}

FileDataResult failedResult(std::string message) {
  FileDataResult result;
  result.error = std::move(message);
  return result;
}

} // namespace

struct FileDataReader::InflateState final {
  ZStream stream{};
  bool initialized = false;
  bool ended = false;
};

FileDataReader::FileDataReader(const std::span<const std::byte> archiveBytes,
                               const Archive& archive, const FileEntry& file,
                               const crypto::Md5Digest& archiveKey) {
  if (archive.dataStart < 0 || file.compressedSize < 0 || file.uncompressedSize < 0) {
    error_ = "invalid NadeoPak file sizes";
    return;
  }

  const std::uint64_t dataOffset = static_cast<std::uint64_t>(archive.dataStart) + file.offset;
  storedSize_ = static_cast<std::size_t>(file.compressedSize);
  outputSize_ = static_cast<std::size_t>(file.uncompressedSize);
  compressed_ = file.compressed();
  useDummyWrite_ = !file.dontUseDummyWrite();
  if (dataOffset > archiveBytes.size()) {
    error_ = "NadeoPak file offset exceeds the archive";
    return;
  }

  const std::size_t offset = static_cast<std::size_t>(dataOffset);
  if (file.encrypted()) {
    constexpr std::size_t kIvSize = 8;
    const std::size_t paddedSize = (storedSize_ + 7U) & ~std::size_t{7U};
    if (archiveBytes.size() - offset < kIvSize ||
        archiveBytes.size() - offset - kIvSize < paddedSize) {
      error_ = "truncated encrypted NadeoPak file";
      return;
    }
    const std::uint64_t initializationVector = readLeU64(archiveBytes.data() + offset);
    cipher_.emplace(archiveBytes.subspan(offset + kIvSize, paddedSize), archiveKey,
                    initializationVector);
  } else {
    if (archiveBytes.size() - offset < storedSize_) {
      error_ = "truncated NadeoPak file";
      return;
    }
    rawStored_ = archiveBytes.subspan(offset, storedSize_);
  }

  if (!compressed_) {
    if (storedSize_ < outputSize_) {
      error_ = "NadeoPak stored size is smaller than the file size";
    }
    return;
  }

  inflate_ = std::make_unique<InflateState>();
  const int status =
      inflateInit_(&inflate_->stream, zlibVersion(), static_cast<int>(sizeof(ZStream)));
  if (status != kZlibOk) {
    error_ = "unable to initialize zlib for NadeoPak file";
    inflate_.reset();
    return;
  }
  inflate_->initialized = true;
}

FileDataReader::~FileDataReader() {
  if (inflate_ != nullptr && inflate_->initialized) {
    inflateEnd(&inflate_->stream);
  }
}

bool FileDataReader::fail(std::string message) {
  if (error_.empty()) {
    error_ = std::move(message);
  }
  return false;
}

bool FileDataReader::readStored(const std::span<std::byte> destination) {
  if (destination.size() > storedSize_ - storedPosition_) {
    return fail("read exceeds the stored NadeoPak file size");
  }

  if (cipher_.has_value()) {
    if (!cipher_->readBytes(destination)) {
      return fail("unable to decrypt NadeoPak file");
    }
  } else {
    std::copy_n(rawStored_.begin() + static_cast<std::ptrdiff_t>(storedPosition_),
                destination.size(), destination.begin());
  }
  storedPosition_ += destination.size();
  return true;
}

bool FileDataReader::refillInflateInput() {
  if (inflate_ == nullptr || inflate_->stream.availableIn != 0) {
    return true;
  }
  if (storedPosition_ == storedSize_) {
    return fail("compressed NadeoPak file ended before the zlib stream");
  }

  std::size_t count = std::min(inflateInput_.size(), storedSize_ - storedPosition_);
  if (cipher_.has_value()) {
    count = std::min(count, cipher_->cycleBytesRemaining());
  }
  if (!readStored(std::span(inflateInput_).first(count))) {
    return false;
  }
  inflate_->stream.nextIn = reinterpret_cast<ZByte*>(inflateInput_.data());
  inflate_->stream.availableIn = static_cast<ZUInt>(count);
  return true;
}

bool FileDataReader::refillInflateOutput() {
  if (inflateOutputPosition_ != inflateOutputSize_) {
    return true;
  }
  inflateOutputPosition_ = 0;
  inflateOutputSize_ = 0;

  while (inflateOutputSize_ < inflateOutput_.size() && !inflate_->ended) {
    if (inflate_->stream.availableIn == 0 && !refillInflateInput()) {
      return false;
    }

    const std::size_t chunkSize = inflateOutput_.size() - inflateOutputSize_;
    inflate_->stream.nextOut = reinterpret_cast<ZByte*>(
        inflateOutput_.data() + static_cast<std::ptrdiff_t>(inflateOutputSize_));
    inflate_->stream.availableOut = static_cast<ZUInt>(chunkSize);
    const ZUInt inputBefore = inflate_->stream.availableIn;
    const ZULong outputBefore = inflate_->stream.totalOut;
    const int status = inflate(&inflate_->stream, kZlibNoFlush);
    const std::size_t produced = chunkSize - inflate_->stream.availableOut;
    inflateOutputSize_ += produced;

    if (status == kZlibStreamEnd) {
      inflate_->ended = true;
    } else if (status != kZlibOk) {
      const char* const detail = inflate_->stream.message;
      return fail(detail == nullptr ? "unable to inflate NadeoPak file"
                                    : std::string("unable to inflate NadeoPak file: ") + detail);
    }
    if (inputBefore == inflate_->stream.availableIn && outputBefore == inflate_->stream.totalOut) {
      return fail("zlib made no progress while reading NadeoPak file");
    }
  }

  if (inflateProduced_ > outputSize_ || inflateOutputSize_ > outputSize_ - inflateProduced_) {
    return fail("zlib stream exceeds the declared NadeoPak file size");
  }
  inflateProduced_ += inflateOutputSize_;
  if (inflate_->ended && inflateProduced_ != outputSize_) {
    return fail("zlib stream is shorter than the declared NadeoPak file size");
  }
  if (inflateOutputSize_ == 0) {
    return fail("zlib produced no NadeoPak file data");
  }
  return true;
}

bool FileDataReader::readInflated(const std::span<std::byte> destination) {
  std::size_t written = 0;
  while (written < destination.size()) {
    if (!refillInflateOutput()) {
      return false;
    }
    const std::size_t count =
        std::min(destination.size() - written, inflateOutputSize_ - inflateOutputPosition_);
    std::copy_n(inflateOutput_.begin() + static_cast<std::ptrdiff_t>(inflateOutputPosition_), count,
                destination.begin() + static_cast<std::ptrdiff_t>(written));
    inflateOutputPosition_ += count;
    written += count;
  }
  return true;
}

bool FileDataReader::validateInflateEnd() {
  while (!inflate_->ended) {
    if (inflate_->stream.availableIn == 0 && !refillInflateInput()) {
      return false;
    }

    std::byte extra{};
    inflate_->stream.nextOut = reinterpret_cast<ZByte*>(&extra);
    inflate_->stream.availableOut = 1;
    const ZUInt inputBefore = inflate_->stream.availableIn;
    const int status = inflate(&inflate_->stream, kZlibNoFlush);
    if (inflate_->stream.availableOut == 0) {
      return fail("zlib stream exceeds the declared NadeoPak file size");
    }
    if (status == kZlibStreamEnd) {
      inflate_->ended = true;
    } else if (status != kZlibOk) {
      const char* const detail = inflate_->stream.message;
      return fail(detail == nullptr
                      ? "unable to finish NadeoPak zlib stream"
                      : std::string("unable to finish NadeoPak zlib stream: ") + detail);
    }
    if (inputBefore == inflate_->stream.availableIn) {
      return fail("zlib made no progress while finishing NadeoPak file");
    }
  }
  return true;
}

bool FileDataReader::readBytes(const std::span<std::byte> destination) {
  if (!error_.empty()) {
    return false;
  }
  if (destination.size() > outputSize_ - outputPosition_) {
    return fail("read exceeds the uncompressed NadeoPak file size");
  }
  if (destination.empty()) {
    return true;
  }

  const bool success = compressed_ ? readInflated(destination) : readStored(destination);
  if (!success) {
    return false;
  }
  outputPosition_ += destination.size();
  return true;
}

void FileDataReader::initialize(const std::span<const std::byte> bytes) noexcept {
  if (useDummyWrite_ && cipher_.has_value()) {
    cipher_->initialize(bytes);
  }
}

bool FileDataReader::finish() {
  if (!error_.empty()) {
    return false;
  }
  if (outputPosition_ != outputSize_) {
    return fail("cannot finish NadeoPak input before all logical bytes are read");
  }
  return !compressed_ || validateInflateEnd();
}

FileDataResult readFileData(const std::span<const std::byte> archiveBytes, const Archive& archive,
                            const FileEntry& file, const crypto::Md5Digest& archiveKey) {
  FileDataReader reader(archiveBytes, archive, file, archiveKey);
  if (!reader) {
    return failedResult(std::string(reader.error()));
  }

  FileDataResult result;
  result.data.resize(reader.size());
  if (!reader.readBytes(result.data)) {
    return failedResult(std::string(reader.error()));
  }
  if (!reader.finish()) {
    return failedResult(std::string(reader.error()));
  }
  return result;
}

FileDataResult readStoredFileDataWithoutDummyWrites(const std::span<const std::byte> archiveBytes,
                                                    const Archive& archive, const FileEntry& file,
                                                    const crypto::Md5Digest& archiveKey) {
  if (archive.dataStart < 0 || file.compressedSize < 0) {
    return failedResult("invalid NadeoPak file sizes");
  }
  const std::uint64_t dataOffset = static_cast<std::uint64_t>(archive.dataStart) + file.offset;
  const std::size_t storedSize = static_cast<std::size_t>(file.compressedSize);
  if (dataOffset > archiveBytes.size()) {
    return failedResult("NadeoPak file offset exceeds the archive");
  }

  const std::size_t offset = static_cast<std::size_t>(dataOffset);
  FileDataResult result;
  result.data.resize(storedSize);
  if (!file.encrypted()) {
    if (archiveBytes.size() - offset < storedSize) {
      return failedResult("truncated NadeoPak file");
    }
    std::copy_n(archiveBytes.begin() + static_cast<std::ptrdiff_t>(offset), storedSize,
                result.data.begin());
    return result;
  }

  constexpr std::size_t kIvSize = 8;
  const std::size_t paddedSize = (storedSize + 7U) & ~std::size_t{7U};
  if (archiveBytes.size() - offset < kIvSize ||
      archiveBytes.size() - offset - kIvSize < paddedSize) {
    return failedResult("truncated encrypted NadeoPak file");
  }
  const std::uint64_t initializationVector = readLeU64(archiveBytes.data() + offset);
  CipherReader reader(archiveBytes.subspan(offset + kIvSize, paddedSize), archiveKey,
                      initializationVector);
  if (!reader.readBytes(result.data)) {
    return failedResult("unable to decrypt NadeoPak file");
  }
  return result;
}

} // namespace tmnf::pak
