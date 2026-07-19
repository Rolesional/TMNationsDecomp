#include "tmnf/gameplay/race_start.hpp"

#include <cmath>
#include <cstdlib>

namespace {

bool closeEnough(const double actual, const double expected) {
  return std::abs(actual - expected) < 1.0e-9;
}

} // namespace

int main() {
  using tmnf::gameplay::RaceStartClock;
  using tmnf::gameplay::RaceStartCue;
  using tmnf::gameplay::RaceStartPhase;
  using tmnf::gameplay::RaceStartState;

  RaceStartClock clock;
  auto frame = clock.current();
  if (frame.phase != RaceStartPhase::Hidden || frame.controlsReleased ||
      !closeEnough(frame.raceTimeSeconds, 0.0)) {
    return EXIT_FAILURE;
  }

  frame = clock.advance(0.001);
  if (frame.phase != RaceStartPhase::Countdown3 || !frame.hasCue(RaceStartCue::Countdown3) ||
      frame.controlsReleased || !closeEnough(frame.simulationSeconds, 0.0)) {
    return EXIT_FAILURE;
  }

  if (!clock.restore(RaceStartState{0.833})) {
    return EXIT_FAILURE;
  }
  frame = clock.advance(0.001);
  if (frame.phase != RaceStartPhase::Countdown2 || !frame.hasCue(RaceStartCue::Countdown2)) {
    return EXIT_FAILURE;
  }

  if (!clock.restore(RaceStartState{1.666})) {
    return EXIT_FAILURE;
  }
  frame = clock.advance(0.001);
  if (frame.phase != RaceStartPhase::Countdown1 || !frame.hasCue(RaceStartCue::Countdown1)) {
    return EXIT_FAILURE;
  }

  if (!clock.restore(RaceStartState{2.499})) {
    return EXIT_FAILURE;
  }
  frame = clock.advance(0.001);
  if (frame.phase != RaceStartPhase::Go || !frame.hasCue(RaceStartCue::Go) ||
      !frame.controlsReleased || !closeEnough(frame.simulationSeconds, 0.0) ||
      !closeEnough(frame.raceTimeSeconds, 0.0)) {
    return EXIT_FAILURE;
  }

  frame = clock.advance(0.001);
  if (frame.phase != RaceStartPhase::Go || !frame.controlsReleased ||
      !closeEnough(frame.simulationSeconds, 0.001) ||
      !closeEnough(frame.raceTimeSeconds, 0.001)) {
    return EXIT_FAILURE;
  }

  if (!clock.restore(RaceStartState{2.999})) {
    return EXIT_FAILURE;
  }
  frame = clock.advance(0.001);
  if (frame.phase != RaceStartPhase::Hidden || !frame.controlsReleased) {
    return EXIT_FAILURE;
  }

  clock.reset();
  frame = clock.advance(2.6);
  if (!frame.hasCue(RaceStartCue::Countdown3) || !frame.hasCue(RaceStartCue::Countdown2) ||
      !frame.hasCue(RaceStartCue::Countdown1) || !frame.hasCue(RaceStartCue::Go) ||
      !closeEnough(frame.simulationSeconds, 0.1)) {
    return EXIT_FAILURE;
  }

  if (clock.restore(RaceStartState{-1.0})) {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
