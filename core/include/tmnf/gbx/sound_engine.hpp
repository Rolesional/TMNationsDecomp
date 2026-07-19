#pragma once

#include "tmnf/gbx/vehicle_tunings.hpp"

#include <array>
#include <cstddef>
#include <string>
#include <vector>

namespace tmnf::gbx {

enum class SoundEngineInput : std::size_t {
  Speed,
  Distance,
  Rpm,
  Acceleration,
  Count,
};

struct SoundEngineComponent final {
  std::string soundPath;
  float minVolume = 0.0F;
  float maxVolume = 0.0F;
  float fadeInStartRpm = 0.0F;
  float fadeInEndRpm = 0.0F;
  float fadeOutStartRpm = 0.0F;
  float fadeOutEndRpm = 0.0F;
  float minPitch = 0.0F;
  float maxPitch = 0.0F;
  float pitchShiftStartRpm = 0.0F;
  float pitchShiftEndRpm = 0.0F;
};

struct SoundEngine final {
  std::vector<SoundEngineComponent> components;
  float volume = 1.0F;
  float maxRpm = 0.0F;
  std::array<RealCurve, static_cast<std::size_t>(SoundEngineInput::Count)> volumeCurves;
  std::array<RealCurve, static_cast<std::size_t>(SoundEngineInput::Count)> alphaCurves;
};

struct SoundEngineParseResult final {
  SoundEngine engine;
  std::string error;

  [[nodiscard]] explicit operator bool() const noexcept { return error.empty(); }
};

struct SoundEngineComponentMix final {
  float volume = 0.0F;
  float pitch = 1.0F;
};

// Parses the legacy TMNF CPlugSoundEngine stream with its parser-timed
// component and embedded-curve dummy writes.
[[nodiscard]] SoundEngineParseResult parseSoundEngine(Input& input);

[[nodiscard]] SoundEngineComponentMix
evaluateSoundEngineComponent(const SoundEngineComponent& component, float signedRpm) noexcept;

[[nodiscard]] float evaluateSoundEngineVolume(const SoundEngine& engine, float speed,
                                              float distance, float signedRpm,
                                              float acceleration) noexcept;

} // namespace tmnf::gbx
