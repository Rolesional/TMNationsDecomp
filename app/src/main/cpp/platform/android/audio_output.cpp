#include "platform/android/audio_output.hpp"

#include <aaudio/AAudio.h>
#include <android/log.h>

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdint>
#include <span>
#include <utility>
#include <vector>

namespace tmnf::android {
namespace {

constexpr char kLogTag[] = "TMNF";
constexpr double kTau = 6.28318530717958647692;
static_assert(std::atomic<std::uint32_t>::is_always_lock_free);
static_assert(std::atomic<std::uint64_t>::is_always_lock_free);

std::uint64_t packFloats(const float first, const float second) noexcept {
  return static_cast<std::uint64_t>(std::bit_cast<std::uint32_t>(first)) |
         (static_cast<std::uint64_t>(std::bit_cast<std::uint32_t>(second)) << 32U);
}

std::pair<float, float> unpackFloats(const std::uint64_t packed) noexcept {
  return {
      std::bit_cast<float>(static_cast<std::uint32_t>(packed)),
      std::bit_cast<float>(static_cast<std::uint32_t>(packed >> 32U)),
  };
}

} // namespace

AudioOutput::AudioOutput() : frames_(131'072) {}

AudioOutput::~AudioOutput() { stop(); }

bool AudioOutput::start() {
  if (stream_ != nullptr) {
    return true;
  }
  pendingError_.store(0, std::memory_order_release);

  AAudioStreamBuilder* builder = nullptr;
  aaudio_result_t result = AAudio_createStreamBuilder(&builder);
  if (result != AAUDIO_OK || builder == nullptr) {
    __android_log_print(ANDROID_LOG_WARN, kLogTag, "AAudio builder failed: %s",
                        AAudio_convertResultToText(result));
    return false;
  }

  AAudioStreamBuilder_setDirection(builder, AAUDIO_DIRECTION_OUTPUT);
  AAudioStreamBuilder_setFormat(builder, AAUDIO_FORMAT_PCM_FLOAT);
  AAudioStreamBuilder_setChannelCount(builder, 2);
  AAudioStreamBuilder_setSampleRate(builder, sampleRate_);
  AAudioStreamBuilder_setSharingMode(builder, AAUDIO_SHARING_MODE_SHARED);
  AAudioStreamBuilder_setPerformanceMode(builder, AAUDIO_PERFORMANCE_MODE_LOW_LATENCY);
  AAudioStreamBuilder_setDataCallback(builder, &AudioOutput::dataCallback, this);
  AAudioStreamBuilder_setErrorCallback(builder, &AudioOutput::errorCallback, this);

  result = AAudioStreamBuilder_openStream(builder, &stream_);
  AAudioStreamBuilder_delete(builder);
  if (result != AAUDIO_OK || stream_ == nullptr) {
    __android_log_print(ANDROID_LOG_WARN, kLogTag, "AAudio stream open failed: %s",
                        AAudio_convertResultToText(result));
    stream_ = nullptr;
    return false;
  }

  if (AAudioStream_getFormat(stream_) != AAUDIO_FORMAT_PCM_FLOAT ||
      AAudioStream_getChannelCount(stream_) != 2) {
    __android_log_print(ANDROID_LOG_WARN, kLogTag, "AAudio did not provide float stereo output");
    AAudioStream_close(stream_);
    stream_ = nullptr;
    return false;
  }
  sampleRate_ = AAudioStream_getSampleRate(stream_);
  frames_.clear();
  decodeRaceSounds();
  decodeEngineSounds();
  result = AAudioStream_requestStart(stream_);
  if (result != AAUDIO_OK) {
    __android_log_print(ANDROID_LOG_WARN, kLogTag, "AAudio start failed: %s",
                        AAudio_convertResultToText(result));
    AAudioStream_close(stream_);
    stream_ = nullptr;
    return false;
  }

  __android_log_print(ANDROID_LOG_INFO, kLogTag, "AAudio ready: %d Hz stereo, burst=%d frames",
                      sampleRate_, AAudioStream_getFramesPerBurst(stream_));
  return true;
}

void AudioOutput::stop() {
  if (stream_ == nullptr) {
    return;
  }
  AAudioStream_requestStop(stream_);
  AAudioStream_close(stream_);
  stream_ = nullptr;
  frames_.clear();
}

void AudioOutput::update() {
  const std::int32_t error = pendingError_.exchange(0, std::memory_order_acq_rel);
  if (error == 0) {
    return;
  }
  __android_log_print(ANDROID_LOG_WARN, kLogTag, "AAudio disconnected: %s; reopening",
                      AAudio_convertResultToText(error));
  stop();
  start();
}

void AudioOutput::setSound(const RaceSound sound, std::vector<std::byte> wavBytes) {
  soundBytes_[static_cast<std::size_t>(sound)] = std::move(wavBytes);
  if (stream_ != nullptr) {
    decodeRaceSounds();
  }
}

void AudioOutput::setEngine(tmnf::gbx::SoundEngine engine,
                            std::vector<std::vector<std::byte>> wavBytes) {
  if (!engine_.components.empty()) {
    return;
  }
  if (engine.components.empty() || engine.components.size() != wavBytes.size()) {
    __android_log_print(ANDROID_LOG_WARN, kLogTag, "Engine graph/WAV count mismatch");
    return;
  }
  const bool restart = stream_ != nullptr;
  if (restart) {
    stop();
  }
  engine_ = std::move(engine);
  engineSoundBytes_ = std::move(wavBytes);
  if (restart) {
    start();
  }
}

void AudioOutput::setEngineState(float speedInput, float signedRpm, float acceleration) noexcept {
  if (!std::isfinite(speedInput)) speedInput = 0.0F;
  if (!std::isfinite(signedRpm)) signedRpm = 0.0F;
  if (!std::isfinite(acceleration)) acceleration = 0.0F;
  engineRpmAcceleration_.store(packFloats(signedRpm, acceleration),
                               std::memory_order_release);
  engineSpeedInput_.store(std::bit_cast<std::uint32_t>(speedInput), std::memory_order_release);
}

void AudioOutput::playSound(const RaceSound sound) {
  const std::size_t index = static_cast<std::size_t>(sound);
  if (index >= sounds_.size() || sounds_[index].empty()) {
    return;
  }
  const std::size_t queued = frames_.write(sounds_[index]);
  __android_log_print(ANDROID_LOG_DEBUG, kLogTag, "Queued %zu/%zu race-sound frames", queued,
                      sounds_[index].size());
}

void AudioOutput::decodeRaceSounds() {
  for (std::size_t index = 0; index < sounds_.size(); ++index) {
    sounds_[index].clear();
    if (soundBytes_[index].empty()) {
      continue;
    }
    tmnf::platform::AudioClipDecodeResult decoded =
        tmnf::platform::decodePcm16Wav(soundBytes_[index], sampleRate_);
    if (!decoded) {
      __android_log_print(ANDROID_LOG_WARN, kLogTag, "Race sound %zu decode failed: %s", index,
                          decoded.error.c_str());
      continue;
    }
    sounds_[index] = std::move(decoded.frames);
    __android_log_print(ANDROID_LOG_INFO, kLogTag, "Race sound %zu ready: %zu frames at %d Hz",
                        index, sounds_[index].size(), sampleRate_);
  }
}

void AudioOutput::decodeEngineSounds() {
  engineSounds_.clear();
  engineSounds_.resize(engineSoundBytes_.size());
  engineCursors_.assign(engineSoundBytes_.size(), 0.0);
  for (std::size_t index = 0; index < engineSoundBytes_.size(); ++index) {
    tmnf::platform::AudioClipDecodeResult decoded =
        tmnf::platform::decodePcm16Wav(engineSoundBytes_[index], sampleRate_);
    if (!decoded) {
      __android_log_print(ANDROID_LOG_WARN, kLogTag, "Engine sound %zu decode failed: %s", index,
                          decoded.error.c_str());
      continue;
    }
    engineSounds_[index] = std::move(decoded.frames);
  }
  if (!engineSounds_.empty()) {
    __android_log_print(ANDROID_LOG_INFO, kLogTag, "Engine sound ready: %zu continuous layers",
                        engineSounds_.size());
  }
}

void AudioOutput::playTone(const float frequencyHz, const float durationSeconds) {
  if (sampleRate_ <= 0) {
    return;
  }
  const float frequency = std::clamp(frequencyHz, 20.0F, 20'000.0F);
  const float duration = std::clamp(durationSeconds, 0.0F, 0.5F);
  const std::size_t frameCount =
      static_cast<std::size_t>(static_cast<float>(sampleRate_) * duration);
  if (frameCount == 0) {
    return;
  }

  std::vector<tmnf::platform::StereoFrame> tone(frameCount);
  const std::size_t fadeFrames =
      std::min<std::size_t>(static_cast<std::size_t>(sampleRate_ / 200), frameCount / 2);
  const double phaseStep = kTau * static_cast<double>(frequency) / static_cast<double>(sampleRate_);
  for (std::size_t index = 0; index < frameCount; ++index) {
    float envelope = 1.0F;
    if (fadeFrames != 0) {
      envelope =
          std::min({1.0F, static_cast<float>(index) / static_cast<float>(fadeFrames),
                    static_cast<float>(frameCount - index - 1) / static_cast<float>(fadeFrames)});
    }
    const float sample = 0.15F * envelope * static_cast<float>(std::sin(tonePhase_));
    tone[index] = tmnf::platform::StereoFrame{sample, sample};
    tonePhase_ += phaseStep;
    if (tonePhase_ >= kTau) {
      tonePhase_ -= kTau;
    }
  }
  const std::size_t queued = frames_.write(tone);
  __android_log_print(ANDROID_LOG_DEBUG, kLogTag, "Queued %zu/%zu confirm-tone frames", queued,
                      tone.size());
}

aaudio_data_callback_result_t AudioOutput::dataCallback(AAudioStream*, void* const userData,
                                                        void* const audioData,
                                                        const std::int32_t frameCount) {
  static_assert(sizeof(tmnf::platform::StereoFrame) == sizeof(float) * 2);
  auto& output = *static_cast<AudioOutput*>(userData);
  std::span frames(static_cast<tmnf::platform::StereoFrame*>(audioData),
                   static_cast<std::size_t>(frameCount));
  const std::size_t read = output.frames_.read(frames);
  std::fill(frames.begin() + static_cast<std::ptrdiff_t>(read), frames.end(),
            tmnf::platform::StereoFrame{});
  const auto [rpm, acceleration] = unpackFloats(
      output.engineRpmAcceleration_.load(std::memory_order_acquire));
  const float speedInput =
      std::bit_cast<float>(output.engineSpeedInput_.load(std::memory_order_acquire));
  const float rootVolume = output.engine_.volume *
                           tmnf::gbx::evaluateSoundEngineVolume(
                               output.engine_, speedInput, 0.0F, rpm, acceleration);
  const std::size_t componentCount =
      std::min({output.engine_.components.size(), output.engineSounds_.size(),
                output.engineCursors_.size()});
  for (std::size_t componentIndex = 0; componentIndex < componentCount; ++componentIndex) {
    const tmnf::gbx::SoundEngineComponentMix mix = tmnf::gbx::evaluateSoundEngineComponent(
        output.engine_.components[componentIndex], rpm);
    const float rawGain = rootVolume * mix.volume;
    const float gain = std::isfinite(rawGain) && rawGain > 0.0F ? rawGain : 0.0F;
    for (tmnf::platform::StereoFrame& frame : frames) {
      const tmnf::platform::StereoFrame engineFrame = tmnf::platform::sampleAudioLoop(
          output.engineSounds_[componentIndex], output.engineCursors_[componentIndex], mix.pitch);
      frame.left += engineFrame.left * gain;
      frame.right += engineFrame.right * gain;
    }
  }
  for (tmnf::platform::StereoFrame& frame : frames) {
    frame.left = std::isfinite(frame.left) ? std::clamp(frame.left, -1.0F, 1.0F) : 0.0F;
    frame.right = std::isfinite(frame.right) ? std::clamp(frame.right, -1.0F, 1.0F) : 0.0F;
  }
  return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

void AudioOutput::errorCallback(AAudioStream*, void* const userData, const aaudio_result_t error) {
  static_cast<AudioOutput*>(userData)->pendingError_.store(error, std::memory_order_release);
}

} // namespace tmnf::android
