#include "tmnf/platform/audio.hpp"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <string_view>
#include <utility>

namespace tmnf::platform {
namespace {

AudioClipDecodeResult decodeFailed(std::string message) {
  AudioClipDecodeResult result;
  result.error = std::move(message);
  return result;
}

bool hasId(const std::span<const std::byte> bytes, const std::size_t offset,
           const std::string_view id) noexcept {
  if (id.size() != 4U || offset > bytes.size() || 4U > bytes.size() - offset) {
    return false;
  }
  for (std::size_t index = 0; index < 4U; ++index) {
    if (std::to_integer<unsigned char>(bytes[offset + index]) !=
        static_cast<unsigned char>(id[index])) {
      return false;
    }
  }
  return true;
}

std::uint16_t readU16(const std::span<const std::byte> bytes, const std::size_t offset) noexcept {
  return static_cast<std::uint16_t>(std::to_integer<std::uint8_t>(bytes[offset])) |
         static_cast<std::uint16_t>(std::to_integer<std::uint8_t>(bytes[offset + 1U]) << 8U);
}

std::uint32_t readU32(const std::span<const std::byte> bytes, const std::size_t offset) noexcept {
  std::uint32_t value = 0;
  for (std::size_t index = 0; index < 4U; ++index) {
    value |= static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(bytes[offset + index]))
             << (index * 8U);
  }
  return value;
}

float readSample(const std::span<const std::byte> data, const std::size_t frame,
                 const std::size_t channel, const std::size_t blockAlign) noexcept {
  const std::size_t offset = frame * blockAlign + channel * 2U;
  const std::int16_t sample = std::bit_cast<std::int16_t>(readU16(data, offset));
  return static_cast<float>(sample) / 32768.0F;
}

} // namespace

AudioClipDecodeResult decodePcm16Wav(const std::span<const std::byte> bytes,
                                     const std::int32_t outputSampleRate) {
  if (outputSampleRate <= 0 || outputSampleRate > 384'000 || bytes.size() < 12U ||
      !hasId(bytes, 0, "RIFF") || !hasId(bytes, 8, "WAVE")) {
    return decodeFailed("invalid RIFF/WAVE header or output sample rate");
  }

  std::optional<std::span<const std::byte>> format;
  std::optional<std::span<const std::byte>> data;
  for (std::size_t offset = 12U; offset + 8U <= bytes.size();) {
    const std::uint32_t chunkSize = readU32(bytes, offset + 4U);
    const std::size_t payloadOffset = offset + 8U;
    if (chunkSize > bytes.size() - payloadOffset) {
      return decodeFailed("RIFF chunk exceeds the WAV input");
    }
    const std::span payload = bytes.subspan(payloadOffset, chunkSize);
    if (!format.has_value() && hasId(bytes, offset, "fmt ")) {
      format = payload;
    } else if (!data.has_value() && hasId(bytes, offset, "data")) {
      data = payload;
    }
    const std::size_t paddedSize = static_cast<std::size_t>(chunkSize) + (chunkSize & 1U);
    if (paddedSize > bytes.size() - payloadOffset) {
      return decodeFailed("RIFF chunk padding exceeds the WAV input");
    }
    offset = payloadOffset + paddedSize;
  }
  if (!format.has_value() || format->size() < 16U || !data.has_value()) {
    return decodeFailed("WAV is missing its PCM format or data chunk");
  }

  const std::uint16_t encoding = readU16(*format, 0);
  const std::uint16_t channelCount = readU16(*format, 2);
  const std::uint32_t sourceSampleRate = readU32(*format, 4);
  const std::uint32_t byteRate = readU32(*format, 8);
  const std::uint16_t blockAlign = readU16(*format, 12);
  const std::uint16_t bitsPerSample = readU16(*format, 14);
  const std::uint32_t expectedBlockAlign = static_cast<std::uint32_t>(channelCount) * 2U;
  if (encoding != 1U || (channelCount != 1U && channelCount != 2U) || bitsPerSample != 16U ||
      sourceSampleRate == 0U || sourceSampleRate > 384'000U || blockAlign != expectedBlockAlign ||
      byteRate != sourceSampleRate * expectedBlockAlign || data->empty() ||
      data->size() % blockAlign != 0U) {
    return decodeFailed("unsupported or inconsistent PCM16 WAV format");
  }

  const std::size_t sourceFrameCount = data->size() / blockAlign;
  constexpr std::uint64_t kMaximumOutputFrames = 1ULL << 27U;
  const std::uint64_t outputFrameCount =
      (static_cast<std::uint64_t>(sourceFrameCount) * static_cast<std::uint32_t>(outputSampleRate) +
       sourceSampleRate - 1U) /
      sourceSampleRate;
  if (outputFrameCount == 0U || outputFrameCount > kMaximumOutputFrames ||
      outputFrameCount > std::numeric_limits<std::size_t>::max()) {
    return decodeFailed("resampled WAV output is too large");
  }

  AudioClipDecodeResult result;
  result.frames.resize(static_cast<std::size_t>(outputFrameCount));
  for (std::size_t index = 0; index < result.frames.size(); ++index) {
    const std::uint64_t sourceNumerator = static_cast<std::uint64_t>(index) * sourceSampleRate;
    const std::size_t first = std::min<std::size_t>(
        static_cast<std::size_t>(sourceNumerator / static_cast<std::uint32_t>(outputSampleRate)),
        sourceFrameCount - 1U);
    const std::size_t second = std::min(first + 1U, sourceFrameCount - 1U);
    const float fraction =
        static_cast<float>(sourceNumerator % static_cast<std::uint32_t>(outputSampleRate)) /
        static_cast<float>(outputSampleRate);
    const float firstLeft = readSample(*data, first, 0, blockAlign);
    const float secondLeft = readSample(*data, second, 0, blockAlign);
    const float firstRight = readSample(*data, first, channelCount == 1U ? 0U : 1U, blockAlign);
    const float secondRight = readSample(*data, second, channelCount == 1U ? 0U : 1U, blockAlign);
    result.frames[index] = {
        firstLeft + (secondLeft - firstLeft) * fraction,
        firstRight + (secondRight - firstRight) * fraction,
    };
  }
  return result;
}

StereoFrame sampleAudioLoop(const std::span<const StereoFrame> frames, double& cursor,
                            const float pitch) noexcept {
  if (frames.empty() || !std::isfinite(cursor) || !std::isfinite(pitch) || pitch <= 0.0F) {
    cursor = 0.0;
    return {};
  }
  const double frameCount = static_cast<double>(frames.size());
  if (cursor < 0.0 || cursor >= frameCount) {
    cursor = std::fmod(std::max(cursor, 0.0), frameCount);
  }
  const std::size_t first = static_cast<std::size_t>(cursor);
  const std::size_t second = first + 1U == frames.size() ? 0U : first + 1U;
  const float fraction = static_cast<float>(cursor - static_cast<double>(first));
  const StereoFrame sample{
      std::lerp(frames[first].left, frames[second].left, fraction),
      std::lerp(frames[first].right, frames[second].right, fraction),
  };
  cursor += static_cast<double>(pitch);
  if (cursor >= frameCount) {
    cursor -= frameCount;
    if (cursor >= frameCount) {
      cursor = std::fmod(cursor, frameCount);
    }
  }
  return sample;
}

AudioRingBuffer::AudioRingBuffer(const std::size_t capacityFrames)
    : frames_(std::max<std::size_t>(capacityFrames, 1) + 1) {}

std::size_t AudioRingBuffer::write(const std::span<const StereoFrame> frames) noexcept {
  std::size_t writeIndex = writeIndex_.load(std::memory_order_relaxed);
  const std::size_t readIndex = readIndex_.load(std::memory_order_acquire);
  std::size_t written = 0;
  while (written < frames.size()) {
    const std::size_t next = (writeIndex + 1) % frames_.size();
    if (next == readIndex) {
      break;
    }
    frames_[writeIndex] = frames[written];
    writeIndex = next;
    ++written;
  }
  writeIndex_.store(writeIndex, std::memory_order_release);
  return written;
}

std::size_t AudioRingBuffer::read(const std::span<StereoFrame> frames) noexcept {
  std::size_t readIndex = readIndex_.load(std::memory_order_relaxed);
  const std::size_t writeIndex = writeIndex_.load(std::memory_order_acquire);
  std::size_t read = 0;
  while (read < frames.size() && readIndex != writeIndex) {
    frames[read] = frames_[readIndex];
    readIndex = (readIndex + 1) % frames_.size();
    ++read;
  }
  readIndex_.store(readIndex, std::memory_order_release);
  return read;
}

std::size_t AudioRingBuffer::availableToRead() const noexcept {
  const std::size_t readIndex = readIndex_.load(std::memory_order_acquire);
  const std::size_t writeIndex = writeIndex_.load(std::memory_order_acquire);
  return writeIndex >= readIndex ? writeIndex - readIndex : frames_.size() - readIndex + writeIndex;
}

std::size_t AudioRingBuffer::capacity() const noexcept { return frames_.size() - 1; }

void AudioRingBuffer::clear() noexcept {
  const std::size_t writeIndex = writeIndex_.load(std::memory_order_acquire);
  readIndex_.store(writeIndex, std::memory_order_release);
}

} // namespace tmnf::platform
