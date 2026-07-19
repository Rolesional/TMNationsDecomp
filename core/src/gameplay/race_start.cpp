#include "tmnf/gameplay/race_start.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace tmnf::gameplay {
namespace {

constexpr double kMillisecondsPerSecond = 1'000.0;
constexpr double kCountdownSeconds =
    static_cast<double>(RaceStartClock::kCountdownMilliseconds) / kMillisecondsPerSecond;

std::uint64_t elapsedMilliseconds(const double elapsedSeconds) noexcept {
  return static_cast<std::uint64_t>(std::floor(elapsedSeconds * kMillisecondsPerSecond + 1.0e-9));
}

std::uint8_t cueBit(const RaceStartCue cue) noexcept { return static_cast<std::uint8_t>(cue); }

} // namespace

bool RaceStartAdvance::hasCue(const RaceStartCue cue) const noexcept {
  return (cues & cueBit(cue)) != 0U;
}

void RaceStartClock::reset() noexcept { elapsedSeconds_ = 0.0; }

bool RaceStartClock::restore(const RaceStartState state) noexcept {
  if (!std::isfinite(state.elapsedSeconds) || state.elapsedSeconds < 0.0) {
    return false;
  }
  elapsedSeconds_ = state.elapsedSeconds;
  return true;
}

RaceStartAdvance RaceStartClock::advance(const double deltaSeconds) noexcept {
  if (!std::isfinite(deltaSeconds) || deltaSeconds <= 0.0) {
    return sample();
  }

  const double previousSeconds = elapsedSeconds_;
  const std::uint64_t previousMilliseconds = elapsedMilliseconds(previousSeconds);
  elapsedSeconds_ += deltaSeconds;
  const std::uint64_t currentMilliseconds = elapsedMilliseconds(elapsedSeconds_);
  RaceStartAdvance result = sample();
  if (previousMilliseconds == 0U && currentMilliseconds > 0U) {
    result.cues |= cueBit(RaceStartCue::Countdown3);
  }
  if (previousMilliseconds < 834U && currentMilliseconds >= 834U) {
    result.cues |= cueBit(RaceStartCue::Countdown2);
  }
  if (previousMilliseconds < 1'667U && currentMilliseconds >= 1'667U) {
    result.cues |= cueBit(RaceStartCue::Countdown1);
  }
  if (previousMilliseconds < kCountdownMilliseconds &&
      currentMilliseconds >= kCountdownMilliseconds) {
    result.cues |= cueBit(RaceStartCue::Go);
  }
  result.simulationSeconds = std::max(0.0, elapsedSeconds_ - kCountdownSeconds) -
                             std::max(0.0, previousSeconds - kCountdownSeconds);
  return result;
}

RaceStartAdvance RaceStartClock::current() const noexcept { return sample(); }

RaceStartAdvance RaceStartClock::sample() const noexcept {
  RaceStartAdvance result;
  const std::uint64_t milliseconds = elapsedMilliseconds(elapsedSeconds_);
  if (milliseconds > 0U && milliseconds < kCountdownMilliseconds) {
    switch ((milliseconds * 3U) / kCountdownMilliseconds) {
    case 0U:
      result.phase = RaceStartPhase::Countdown3;
      break;
    case 1U:
      result.phase = RaceStartPhase::Countdown2;
      break;
    default:
      result.phase = RaceStartPhase::Countdown1;
      break;
    }
  } else if (milliseconds >= kCountdownMilliseconds &&
             milliseconds < kCountdownMilliseconds + kGoMilliseconds) {
    result.phase = RaceStartPhase::Go;
  }
  result.controlsReleased = milliseconds >= kCountdownMilliseconds;
  result.raceTimeSeconds = std::max(0.0, elapsedSeconds_ - kCountdownSeconds);
  return result;
}

} // namespace tmnf::gameplay
