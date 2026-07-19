#pragma once

#include <cstdint>

namespace tmnf::gameplay {

enum class RaceStartPhase : std::uint8_t {
  Hidden,
  Countdown3,
  Countdown2,
  Countdown1,
  Go,
};

enum class RaceStartCue : std::uint8_t {
  Countdown3 = 1U << 0U,
  Countdown2 = 1U << 1U,
  Countdown1 = 1U << 2U,
  Go = 1U << 3U,
};

struct RaceStartState final {
  double elapsedSeconds = 0.0;
};

struct RaceStartAdvance final {
  RaceStartPhase phase = RaceStartPhase::Hidden;
  std::uint8_t cues = 0;
  double simulationSeconds = 0.0;
  double raceTimeSeconds = 0.0;
  bool controlsReleased = false;

  [[nodiscard]] bool hasCue(RaceStartCue cue) const noexcept;
};

class RaceStartClock final {
public:
  // CTrackManiaRace1P vslot +0x110, FUN_00715310.
  static constexpr std::uint32_t kCountdownMilliseconds = 2'500U;
  static constexpr std::uint32_t kGoMilliseconds = 500U;

  void reset() noexcept;
  [[nodiscard]] bool restore(RaceStartState state) noexcept;
  [[nodiscard]] RaceStartAdvance advance(double deltaSeconds) noexcept;
  [[nodiscard]] RaceStartAdvance current() const noexcept;
  [[nodiscard]] RaceStartState state() const noexcept { return {elapsedSeconds_}; }

private:
  [[nodiscard]] RaceStartAdvance sample() const noexcept;

  double elapsedSeconds_ = 0.0;
};

} // namespace tmnf::gameplay
