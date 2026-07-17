#include "tmnf/platform/audio.hpp"

#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>
#include <vector>

namespace {

void appendText(std::vector<std::byte>& bytes, const std::string_view text) {
  for (const char character : text) {
    bytes.push_back(static_cast<std::byte>(character));
  }
}

void appendU16(std::vector<std::byte>& bytes, const std::uint16_t value) {
  bytes.push_back(static_cast<std::byte>(value & 0xFFU));
  bytes.push_back(static_cast<std::byte>(value >> 8U));
}

void appendU32(std::vector<std::byte>& bytes, const std::uint32_t value) {
  for (std::size_t index = 0; index < 4U; ++index) {
    bytes.push_back(static_cast<std::byte>((value >> (index * 8U)) & 0xFFU));
  }
}

std::vector<std::byte> makePcm16Wav(const std::uint16_t channels, const std::uint32_t sampleRate,
                                    const std::span<const std::int16_t> samples) {
  std::vector<std::byte> bytes;
  appendText(bytes, "RIFF");
  appendU32(bytes, 0);
  appendText(bytes, "WAVEfmt ");
  appendU32(bytes, 16);
  appendU16(bytes, 1);
  appendU16(bytes, channels);
  appendU32(bytes, sampleRate);
  appendU32(bytes, sampleRate * channels * 2U);
  appendU16(bytes, static_cast<std::uint16_t>(channels * 2U));
  appendU16(bytes, 16);
  appendText(bytes, "data");
  appendU32(bytes, static_cast<std::uint32_t>(samples.size() * 2U));
  for (const std::int16_t sample : samples) {
    appendU16(bytes, std::bit_cast<std::uint16_t>(sample));
  }
  const std::uint32_t riffSize = static_cast<std::uint32_t>(bytes.size() - 8U);
  for (std::size_t index = 0; index < 4U; ++index) {
    bytes[4U + index] = static_cast<std::byte>((riffSize >> (index * 8U)) & 0xFFU);
  }
  return bytes;
}

bool readFile(const std::filesystem::path& path, std::vector<std::byte>& bytes) {
  std::ifstream stream(path, std::ios::binary | std::ios::ate);
  if (!stream) {
    return false;
  }
  const std::streamoff size = stream.tellg();
  if (size < 0) {
    return false;
  }
  bytes.resize(static_cast<std::size_t>(size));
  stream.seekg(0);
  if (!bytes.empty()) {
    stream.read(reinterpret_cast<char*>(bytes.data()), size);
  }
  return static_cast<bool>(stream);
}

} // namespace

int main(const int argc, const char* const* argv) {
  using tmnf::platform::AudioRingBuffer;
  using tmnf::platform::StereoFrame;

  AudioRingBuffer buffer(4);
  const std::array first{StereoFrame{1.0F, -1.0F}, StereoFrame{2.0F, -2.0F},
                         StereoFrame{3.0F, -3.0F}};
  if (buffer.capacity() != 4 || buffer.write(first) != first.size() ||
      buffer.availableToRead() != first.size()) {
    std::cerr << "initial audio write failed\n";
    return 1;
  }

  std::array<StereoFrame, 2> prefix{};
  if (buffer.read(prefix) != prefix.size() || prefix[0].left != 1.0F || prefix[1].right != -2.0F) {
    std::cerr << "initial audio read failed\n";
    return 1;
  }

  const std::array wrapped{StereoFrame{4.0F, -4.0F}, StereoFrame{5.0F, -5.0F},
                           StereoFrame{6.0F, -6.0F}, StereoFrame{7.0F, -7.0F}};
  if (buffer.write(wrapped) != 3 || buffer.availableToRead() != 4) {
    std::cerr << "audio wrap or capacity handling failed\n";
    return 1;
  }

  std::array<StereoFrame, 4> suffix{};
  if (buffer.read(suffix) != suffix.size() || suffix[0].left != 3.0F || suffix[1].left != 4.0F ||
      suffix[3].right != -6.0F || buffer.availableToRead() != 0) {
    std::cerr << "wrapped audio read failed\n";
    return 1;
  }

  if (buffer.write(first) != first.size()) {
    std::cerr << "audio refill before clear failed\n";
    return 1;
  }
  buffer.clear();
  if (buffer.availableToRead() != 0 || buffer.read(prefix) != 0) {
    std::cerr << "audio clear failed\n";
    return 1;
  }

  const std::array monoSamples{std::int16_t{0}, std::int16_t{32767}};
  const std::vector<std::byte> monoWav = makePcm16Wav(1, 2, monoSamples);
  const tmnf::platform::AudioClipDecodeResult mono = tmnf::platform::decodePcm16Wav(monoWav, 4);
  if (!mono || mono.frames.size() != 4U || mono.frames[0].left != 0.0F ||
      std::abs(mono.frames[1].left - 0.49998F) > 0.0001F ||
      std::abs(mono.frames[2].right - 0.99997F) > 0.0001F ||
      mono.frames[1].left != mono.frames[1].right) {
    std::cerr << "mono PCM16 WAV decode/resample failed\n";
    return 1;
  }

  const std::array stereoSamples{std::int16_t{-32768}, std::int16_t{16384}};
  const std::vector<std::byte> stereoWav = makePcm16Wav(2, 4, stereoSamples);
  const tmnf::platform::AudioClipDecodeResult stereo = tmnf::platform::decodePcm16Wav(stereoWav, 4);
  if (!stereo || stereo.frames.size() != 1U || stereo.frames[0].left != -1.0F ||
      stereo.frames[0].right != 0.5F ||
      tmnf::platform::decodePcm16Wav(std::span(monoWav).first(8), 4)) {
    std::cerr << "stereo PCM16 WAV validation failed\n";
    return 1;
  }
  if (argc != 1 && argc != 3) {
    std::cerr << "usage: audio_ring_buffer_test [RaceCheckPoint.wav RaceFinish.wav]\n";
    return 2;
  }
  if (argc == 3) {
    std::vector<std::byte> checkpointBytes;
    std::vector<std::byte> finishBytes;
    if (!readFile(argv[1], checkpointBytes) || !readFile(argv[2], finishBytes)) {
      std::cerr << "unable to read reference race sounds\n";
      return 1;
    }
    const tmnf::platform::AudioClipDecodeResult checkpoint =
        tmnf::platform::decodePcm16Wav(checkpointBytes, 48'000);
    const tmnf::platform::AudioClipDecodeResult finish =
        tmnf::platform::decodePcm16Wav(finishBytes, 48'000);
    if (!checkpoint || checkpoint.frames.size() != 72'281U || !finish ||
        finish.frames.size() != 23'632U) {
      std::cerr << "reference race sounds did not decode at 48 kHz\n";
      return 1;
    }
  }
  return 0;
}
