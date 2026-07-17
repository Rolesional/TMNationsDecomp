#pragma once

#include "tmnf/platform/audio.hpp"

#include <aaudio/AAudio.h>

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace tmnf::android {

enum class RaceSound : std::uint8_t {
  Checkpoint,
  Finish,
  Count,
};

class AudioOutput final {
public:
  AudioOutput();
  ~AudioOutput();

  AudioOutput(const AudioOutput&) = delete;
  AudioOutput& operator=(const AudioOutput&) = delete;

  bool start();
  void stop();
  void update();
  void setSound(RaceSound sound, std::vector<std::byte> wavBytes);
  void playSound(RaceSound sound);
  void playTone(float frequencyHz, float durationSeconds);
  [[nodiscard]] bool ready() const noexcept { return stream_ != nullptr; }

private:
  static aaudio_data_callback_result_t dataCallback(AAudioStream* stream, void* userData,
                                                    void* audioData, std::int32_t frameCount);
  static void errorCallback(AAudioStream* stream, void* userData, aaudio_result_t error);
  void decodeSounds();

  tmnf::platform::AudioRingBuffer frames_;
  std::array<std::vector<std::byte>, static_cast<std::size_t>(RaceSound::Count)> soundBytes_;
  std::array<std::vector<tmnf::platform::StereoFrame>, static_cast<std::size_t>(RaceSound::Count)>
      sounds_;
  AAudioStream* stream_ = nullptr;
  std::atomic<std::int32_t> pendingError_{0};
  std::int32_t sampleRate_ = 48'000;
  double tonePhase_ = 0.0;
};

} // namespace tmnf::android
