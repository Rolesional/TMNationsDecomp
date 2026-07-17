#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace tmnf::platform {

struct StereoFrame final {
  float left = 0.0F;
  float right = 0.0F;
};

struct AudioClipDecodeResult final {
  std::vector<StereoFrame> frames;
  std::string error;

  [[nodiscard]] explicit operator bool() const noexcept { return error.empty(); }
};

// Decodes mono/stereo RIFF PCM16 and linearly resamples it to the output rate.
[[nodiscard]] AudioClipDecodeResult decodePcm16Wav(std::span<const std::byte> bytes,
                                                   std::int32_t outputSampleRate);

// Fixed-capacity single-producer/single-consumer queue. The simulation thread
// writes decoded/mixed frames and the platform audio callback reads them
// without allocation or locks.
class AudioRingBuffer final {
public:
  explicit AudioRingBuffer(std::size_t capacityFrames);

  AudioRingBuffer(const AudioRingBuffer&) = delete;
  AudioRingBuffer& operator=(const AudioRingBuffer&) = delete;

  [[nodiscard]] std::size_t write(std::span<const StereoFrame> frames) noexcept;
  [[nodiscard]] std::size_t read(std::span<StereoFrame> frames) noexcept;
  [[nodiscard]] std::size_t availableToRead() const noexcept;
  [[nodiscard]] std::size_t capacity() const noexcept;
  void clear() noexcept;

private:
  std::vector<StereoFrame> frames_;
  alignas(64) std::atomic<std::size_t> readIndex_{0};
  alignas(64) std::atomic<std::size_t> writeIndex_{0};
};

} // namespace tmnf::platform
