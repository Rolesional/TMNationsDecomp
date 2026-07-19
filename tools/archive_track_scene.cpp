#include "tmnf/content/track_collision.hpp"
#include "tmnf/content/track_scene.hpp"
#include "tmnf/gameplay/vehicle.hpp"
#include "tmnf/gbx/challenge.hpp"
#include "tmnf/gbx/solid.hpp"
#include "tmnf/gbx/sound_engine.hpp"
#include "tmnf/gbx/vehicle_tunings.hpp"
#include "tmnf/pak/vfs.hpp"

#include <algorithm>
#include <array>
#include <cerrno>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

bool readFile(const std::filesystem::path& path, std::vector<std::byte>& output) {
  std::ifstream stream(path, std::ios::binary);
  if (!stream) {
    return false;
  }
  stream.seekg(0, std::ios::end);
  const std::streamoff size = stream.tellg();
  if (size < 0) {
    return false;
  }
  output.resize(static_cast<std::size_t>(size));
  stream.seekg(0, std::ios::beg);
  if (!output.empty()) {
    stream.read(reinterpret_cast<char*>(output.data()), size);
  }
  return static_cast<bool>(stream);
}

std::filesystem::path nativePath(const std::string_view virtualPath) {
  std::filesystem::path result;
  std::size_t begin = 0;
  while (begin < virtualPath.size()) {
    const std::size_t separator = virtualPath.find_first_of("\\/", begin);
    const std::size_t end = separator == std::string_view::npos ? virtualPath.size() : separator;
    if (end != begin) {
      result /= std::string(virtualPath.substr(begin, end - begin));
    }
    if (separator == std::string_view::npos) {
      break;
    }
    begin = separator + 1U;
  }
  return result;
}

std::string jsonEscape(const std::string_view value) {
  std::string result;
  result.reserve(value.size());
  for (const unsigned char character : value) {
    if (character == '"' || character == '\\') {
      result.push_back('\\');
      result.push_back(static_cast<char>(character));
    } else if (character >= 0x20U) {
      result.push_back(static_cast<char>(character));
    }
  }
  return result;
}

void writeFloatArray(const std::vector<float>& values) {
  std::cout << '[';
  for (std::size_t index = 0; index < values.size(); ++index) {
    std::cout << values[index] << (index + 1U == values.size() ? "" : ",");
  }
  std::cout << ']';
}

template <std::size_t Size>
void writeCurves(const std::array<tmnf::gbx::RealCurve, Size>& curves) {
  std::cout << '[';
  for (std::size_t index = 0; index < curves.size(); ++index) {
    std::cout << "{\"x\":";
    writeFloatArray(curves[index].x);
    std::cout << ",\"y\":";
    writeFloatArray(curves[index].y);
    std::cout << '}';
    if (index + 1U != curves.size()) {
      std::cout << ',';
    }
  }
  std::cout << ']';
}

bool finiteVehicleState(const tmnf::gameplay::VehicleState& state) {
  return std::isfinite(state.position.x) && std::isfinite(state.position.y) &&
         std::isfinite(state.position.z) && std::isfinite(state.groundNormal.x) &&
         std::isfinite(state.groundNormal.y) && std::isfinite(state.groundNormal.z) &&
         std::ranges::all_of(state.orientationWxyz,
                             [](const float value) { return std::isfinite(value); }) &&
         std::isfinite(state.angularVelocity.x) && std::isfinite(state.angularVelocity.y) &&
         std::isfinite(state.angularVelocity.z) &&
         std::isfinite(state.priorLocalForcePerInertialMass.x) &&
         std::isfinite(state.priorLocalForcePerInertialMass.y) &&
         std::isfinite(state.priorLocalForcePerInertialMass.z) &&
         std::isfinite(state.smoothedSteering) &&
         std::ranges::all_of(state.wheels, [](const tmnf::gameplay::VehicleWheelState& wheel) {
            return std::isfinite(wheel.suspensionLength) &&
                   std::isfinite(wheel.suspensionSpeed) &&
                   std::isfinite(wheel.pendingCorrection) &&
                   std::isfinite(wheel.contactNormal.x) &&
                   std::isfinite(wheel.contactNormal.y) &&
                   std::isfinite(wheel.contactNormal.z) &&
                   std::isfinite(wheel.contactPoint.x) &&
                   std::isfinite(wheel.contactPoint.y) &&
                   std::isfinite(wheel.contactPoint.z);
         }) &&
         std::isfinite(state.horizontalVelocityX) &&
         std::isfinite(state.horizontalVelocityZ) && std::isfinite(state.verticalVelocity) &&
         std::isfinite(state.heading) && std::isfinite(state.speed) &&
         std::isfinite(state.engineRpm) && std::isfinite(state.engineTargetRpm) &&
         std::isfinite(state.gearShiftSeconds) && std::isfinite(state.raceTimeSeconds);
}

struct DriveEvent final {
  double timeSeconds = 0.0;
  tmnf::gameplay::VehicleControl control;
};

struct DriveResult final {
  std::vector<std::uint32_t> checkpointSequence;
  std::vector<float> checkpointTimes;
  std::vector<float> checkpointPositions;
  std::vector<std::size_t> checkpointRouteProgress;
  tmnf::gameplay::VehicleState state;
  float maxStepDistance = 0.0F;
  double simulatedSeconds = 0.0;
  std::size_t routeProgress = 0;
  double firstAirSeconds = -1.0;
  std::size_t firstAirRouteProgress = 0;
  tmnf::gbx::Float3 firstAirPosition;
  tmnf::gbx::Float3 firstAirVelocity;
  tmnf::gbx::Float3 firstAirAngularVelocity;
  std::array<float, 4> firstAirOrientation{};
  float firstAirHeading = 0.0F;
  float firstAirSpeed = 0.0F;
  std::vector<float> airTransitions;
  std::vector<float> routeMilestones;
  std::size_t resetCount = 0;
  double firstResetSeconds = -1.0;
  std::size_t firstResetRouteProgress = 0;
  tmnf::gbx::Float3 firstResetFrom;
  tmnf::gbx::Float3 firstResetTo;
  double firstResetLastGroundedSeconds = -1.0;
  std::size_t firstResetLastGroundedRouteProgress = 0;
  tmnf::gbx::Float3 firstResetLastGroundedPosition;
  std::array<float, 4> firstResetOrientation{};
  tmnf::gbx::Float3 firstResetAngularVelocity;
  float firstResetSpeed = 0.0F;
  bool finite = false;
  std::string error;
};

DriveResult runDrive(const tmnf::content::TrackScene& scene,
                     const tmnf::content::TrackCollision& collision,
                     const tmnf::gbx::VehicleTuning& tuning,
                     const tmnf::gbx::SolidModel& vehicleSolid,
                     const std::vector<DriveEvent>& events) {
  DriveResult result;
  tmnf::gameplay::VehicleSimulation vehicle;
  const tmnf::gameplay::VehicleInitializeResult initialized =
      vehicle.initialize(scene, collision, tuning, &vehicleSolid);
  if (!initialized) {
    result.error = initialized.error;
    return result;
  }
  constexpr double kDurationSeconds = 30.0;
  constexpr double kStepSeconds = 1.0 / 120.0;
  constexpr double kTimeEpsilon = 1.0e-9;
  tmnf::gameplay::VehicleControl control;
  std::size_t eventIndex = 0;
  std::uint64_t stepIndex = 1;
  double time = 0.0;
  double nextSampleSeconds = 0.0;
  while (time < kDurationSeconds && !vehicle.state().finished) {
    while (eventIndex < events.size() && events[eventIndex].timeSeconds <= time + kTimeEpsilon) {
      control = events[eventIndex].control;
      ++eventIndex;
    }
    while (static_cast<double>(stepIndex) * kStepSeconds <= time + kTimeEpsilon) {
      ++stepIndex;
    }
    double target = std::min(kDurationSeconds, static_cast<double>(stepIndex) * kStepSeconds);
    if (eventIndex < events.size()) {
      target = std::min(target, events[eventIndex].timeSeconds);
    }
    if (target <= time + kTimeEpsilon) {
      continue;
    }
    const tmnf::gameplay::VehicleState before = vehicle.state();
    vehicle.update(static_cast<float>(target - time), control);
    time = target;
    const tmnf::gameplay::VehicleState& after = vehicle.state();
    if (time + kTimeEpsilon >= nextSampleSeconds) {
      result.routeMilestones.insert(
          result.routeMilestones.end(),
          {static_cast<float>(time), static_cast<float>(nextSampleSeconds * 10.0),
           after.position.x, after.position.y, after.position.z,
           after.horizontalVelocityX, after.verticalVelocity, after.horizontalVelocityZ,
           after.heading, after.speed, after.smoothedSteering, control.steering,
           control.throttle, control.brake});
      nextSampleSeconds += 0.1;
    }
    const float dx = after.position.x - before.position.x;
    const float dy = after.position.y - before.position.y;
    const float dz = after.position.z - before.position.z;
    result.maxStepDistance =
        std::max(result.maxStepDistance, std::sqrt(dx * dx + dy * dy + dz * dz));
    for (std::uint32_t checkpoint = before.checkpointCount + 1U;
         checkpoint <= after.checkpointCount; ++checkpoint) {
      result.checkpointSequence.push_back(checkpoint);
      result.checkpointTimes.push_back(after.raceTimeSeconds);
      result.checkpointPositions.insert(result.checkpointPositions.end(),
                                        {after.position.x, after.position.y, after.position.z});
      result.checkpointRouteProgress.push_back(0U);
    }
  }
  result.state = vehicle.state();
  result.simulatedSeconds = time;
  result.finite = finiteVehicleState(result.state);
  return result;
}

float wrapAngle(float value) {
  constexpr float kPi = 3.14159265358979323846F;
  while (value > kPi) {
    value -= 2.0F * kPi;
  }
  while (value < -kPi) {
    value += 2.0F * kPi;
  }
  return value;
}

DriveResult runRoute(const tmnf::content::TrackScene& scene,
                     const tmnf::content::TrackCollision& collision,
                     const tmnf::gbx::VehicleTuning& tuning,
                     const tmnf::gbx::SolidModel& vehicleSolid,
                     const std::vector<tmnf::gbx::Float3>& route) {
  DriveResult result;
  tmnf::gameplay::VehicleSimulation vehicle;
  const tmnf::gameplay::VehicleInitializeResult initialized =
      vehicle.initialize(scene, collision, tuning, &vehicleSolid);
  if (!initialized) {
    result.error = initialized.error;
    return result;
  }
  constexpr float kStepSeconds = 1.0F / 120.0F;
  constexpr double kDurationSeconds = 90.0;
  std::size_t progress = 0;
  double time = 0.0;
  double nextControlSeconds = 0.0;
  double lastGroundedSeconds = 0.0;
  std::size_t lastGroundedRouteProgress = 0;
  tmnf::gbx::Float3 lastGroundedPosition = vehicle.state().position;
  tmnf::gameplay::VehicleControl control;
  std::size_t nextMilestone = 1U;
  while (time < kDurationSeconds && !vehicle.state().finished) {
    const tmnf::gameplay::VehicleState before = vehicle.state();
    if (time + 1.0e-9 >= nextControlSeconds) {
      const std::size_t searchEnd = std::min(route.size(), progress + 80U);
      float nearestDistanceSquared = std::numeric_limits<float>::max();
      std::size_t nearest = progress;
      for (std::size_t index = progress; index < searchEnd; ++index) {
        const float dx = route[index].x - before.position.x;
        const float dz = route[index].z - before.position.z;
        const float distanceSquared = dx * dx + dz * dz;
        if (distanceSquared < nearestDistanceSquared) {
          nearestDistanceSquared = distanceSquared;
          nearest = index;
        }
      }
      progress = std::max(progress, nearest);
      const float lookaheadDistance =
          std::clamp(std::abs(before.speed) * 0.4F, 8.0F, 45.0F);
      std::size_t targetIndex = progress;
      while (targetIndex + 1U < route.size()) {
        const float dx = route[targetIndex].x - before.position.x;
        const float dz = route[targetIndex].z - before.position.z;
        if (dx * dx + dz * dz >= lookaheadDistance * lookaheadDistance) {
          break;
        }
        ++targetIndex;
      }
      const float dx = route[targetIndex].x - before.position.x;
      const float dz = route[targetIndex].z - before.position.z;
      const float headingError = wrapAngle(std::atan2(dx, dz) - before.heading);
      const float absoluteError = std::abs(headingError);
      const std::size_t speedIndex = std::min(targetIndex + 1U, route.size() - 1U);
      const float routeDx = route[speedIndex].x - route[targetIndex].x;
      const float routeDy = route[speedIndex].y - route[targetIndex].y;
      const float routeDz = route[speedIndex].z - route[targetIndex].z;
      const float routeSpeed =
          std::sqrt(routeDx * routeDx + routeDy * routeDy + routeDz * routeDz) * 10.0F;
      const float speedFactor = absoluteError < 0.12F ? 1.05F
                                : absoluteError < 0.3F ? 0.8F
                                : absoluteError < 0.6F ? 0.55F
                                                       : 0.3F;
      const float targetSpeed =
          std::max(12.0F, std::max(50.0F, routeSpeed) * speedFactor);
      const float brake = before.speed > targetSpeed + 1.0F
                              ? std::clamp((before.speed - targetSpeed) / 10.0F, 0.15F, 1.0F)
                              : 0.0F;
      const float steeringGain = progress >= 175U && progress <= 205U ? 3.5F : 2.5F;
      control = {.steering = std::clamp(headingError * steeringGain, -1.0F, 1.0F),
                 .throttle = brake == 0.0F ? 1.0F : 0.0F,
                 .brake = brake};
      nextControlSeconds += 1.0 / 40.0;
    }
    vehicle.update(kStepSeconds, control);
    time += static_cast<double>(kStepSeconds);
    const tmnf::gameplay::VehicleState& after = vehicle.state();
    while (progress >= nextMilestone) {
      result.routeMilestones.insert(
          result.routeMilestones.end(),
          {static_cast<float>(time), static_cast<float>(progress), after.position.x,
           after.position.y, after.position.z, after.horizontalVelocityX,
           after.verticalVelocity, after.horizontalVelocityZ, after.heading, after.speed,
           after.smoothedSteering, control.steering, control.throttle, control.brake});
      ++nextMilestone;
    }
    const float stepX = after.position.x - before.position.x;
    const float stepY = after.position.y - before.position.y;
    const float stepZ = after.position.z - before.position.z;
    const float stepDistance = std::sqrt(stepX * stepX + stepY * stepY + stepZ * stepZ);
    result.maxStepDistance = std::max(result.maxStepDistance, stepDistance);
    if (stepDistance > 32.0F) {
      ++result.resetCount;
      if (result.firstResetSeconds < 0.0) {
        result.firstResetSeconds = time;
        result.firstResetRouteProgress = progress;
        result.firstResetFrom = before.position;
        result.firstResetTo = after.position;
        result.firstResetLastGroundedSeconds = lastGroundedSeconds;
        result.firstResetLastGroundedRouteProgress = lastGroundedRouteProgress;
        result.firstResetLastGroundedPosition = lastGroundedPosition;
        result.firstResetOrientation = before.orientationWxyz;
        result.firstResetAngularVelocity = before.angularVelocity;
        result.firstResetSpeed = before.speed;
      }
    }
    if (after.grounded) {
      lastGroundedSeconds = time;
      lastGroundedRouteProgress = progress;
      lastGroundedPosition = after.position;
    }
    if (result.firstAirSeconds < 0.0 && before.grounded && !after.grounded) {
      result.firstAirSeconds = time;
      result.firstAirRouteProgress = progress;
      result.firstAirPosition = after.position;
      result.firstAirVelocity = {after.horizontalVelocityX, after.verticalVelocity,
                                 after.horizontalVelocityZ};
      result.firstAirAngularVelocity = after.angularVelocity;
      result.firstAirOrientation = after.orientationWxyz;
      result.firstAirHeading = after.heading;
      result.firstAirSpeed = after.speed;
    }
    if (before.grounded && !after.grounded && result.firstResetSeconds < 0.0) {
      result.airTransitions.insert(
          result.airTransitions.end(),
          {static_cast<float>(time), static_cast<float>(progress), after.position.x,
           after.position.y, after.position.z, after.horizontalVelocityX,
           after.verticalVelocity, after.horizontalVelocityZ, after.heading, after.speed});
    }
    for (std::uint32_t checkpoint = before.checkpointCount + 1U;
         checkpoint <= after.checkpointCount; ++checkpoint) {
      result.checkpointSequence.push_back(checkpoint);
      result.checkpointTimes.push_back(after.raceTimeSeconds);
      result.checkpointPositions.insert(result.checkpointPositions.end(),
                                        {after.position.x, after.position.y, after.position.z});
      result.checkpointRouteProgress.push_back(progress);
    }
  }
  result.state = vehicle.state();
  result.simulatedSeconds = time;
  result.routeProgress = progress;
  result.finite = finiteVehicleState(result.state);
  return result;
}

bool sameDrive(const DriveResult& left, const DriveResult& right) {
  return left.checkpointSequence == right.checkpointSequence &&
         left.checkpointTimes == right.checkpointTimes &&
         left.checkpointPositions == right.checkpointPositions &&
         left.checkpointRouteProgress == right.checkpointRouteProgress &&
         left.state.position.x == right.state.position.x &&
         left.state.position.y == right.state.position.y && left.state.position.z == right.state.position.z &&
         left.state.groundNormal.x == right.state.groundNormal.x &&
         left.state.groundNormal.y == right.state.groundNormal.y &&
         left.state.groundNormal.z == right.state.groundNormal.z &&
         left.state.orientationWxyz == right.state.orientationWxyz &&
         left.state.angularVelocity.x == right.state.angularVelocity.x &&
         left.state.angularVelocity.y == right.state.angularVelocity.y &&
         left.state.angularVelocity.z == right.state.angularVelocity.z &&
         left.state.priorLocalForcePerInertialMass.x ==
             right.state.priorLocalForcePerInertialMass.x &&
         left.state.priorLocalForcePerInertialMass.y ==
             right.state.priorLocalForcePerInertialMass.y &&
         left.state.priorLocalForcePerInertialMass.z ==
             right.state.priorLocalForcePerInertialMass.z &&
         left.state.wheels == right.state.wheels &&
         left.state.smoothedSteering == right.state.smoothedSteering &&
         left.state.horizontalVelocityX == right.state.horizontalVelocityX &&
         left.state.horizontalVelocityZ == right.state.horizontalVelocityZ &&
         left.state.verticalVelocity == right.state.verticalVelocity &&
         left.state.heading == right.state.heading && left.state.speed == right.state.speed &&
         left.state.gear == right.state.gear && left.state.engineRpm == right.state.engineRpm &&
         left.state.engineTargetRpm == right.state.engineTargetRpm &&
         left.state.gearShiftSeconds == right.state.gearShiftSeconds &&
         left.state.checkpointCount == right.state.checkpointCount &&
         left.state.completedLaps == right.state.completedLaps &&
         left.state.grounded == right.state.grounded &&
         left.state.finished == right.state.finished &&
         left.state.elapsedSeconds == right.state.elapsedSeconds &&
         left.state.raceTimeSeconds == right.state.raceTimeSeconds &&
         left.maxStepDistance == right.maxStepDistance &&
         left.simulatedSeconds == right.simulatedSeconds &&
         left.routeProgress == right.routeProgress && left.finite == right.finite &&
         left.firstAirSeconds == right.firstAirSeconds &&
         left.firstAirRouteProgress == right.firstAirRouteProgress &&
          left.firstAirPosition.x == right.firstAirPosition.x &&
          left.firstAirPosition.y == right.firstAirPosition.y &&
          left.firstAirPosition.z == right.firstAirPosition.z &&
          left.firstAirVelocity.x == right.firstAirVelocity.x &&
          left.firstAirVelocity.y == right.firstAirVelocity.y &&
          left.firstAirVelocity.z == right.firstAirVelocity.z &&
          left.firstAirAngularVelocity.x == right.firstAirAngularVelocity.x &&
          left.firstAirAngularVelocity.y == right.firstAirAngularVelocity.y &&
          left.firstAirAngularVelocity.z == right.firstAirAngularVelocity.z &&
          left.firstAirOrientation == right.firstAirOrientation &&
          left.firstAirHeading == right.firstAirHeading &&
          left.firstAirSpeed == right.firstAirSpeed &&
          left.airTransitions == right.airTransitions &&
          left.routeMilestones == right.routeMilestones &&
         left.resetCount == right.resetCount &&
         left.firstResetSeconds == right.firstResetSeconds &&
         left.firstResetRouteProgress == right.firstResetRouteProgress &&
         left.firstResetFrom.x == right.firstResetFrom.x &&
         left.firstResetFrom.y == right.firstResetFrom.y &&
         left.firstResetFrom.z == right.firstResetFrom.z &&
         left.firstResetTo.x == right.firstResetTo.x &&
         left.firstResetTo.y == right.firstResetTo.y &&
         left.firstResetTo.z == right.firstResetTo.z &&
         left.firstResetLastGroundedSeconds == right.firstResetLastGroundedSeconds &&
         left.firstResetLastGroundedRouteProgress == right.firstResetLastGroundedRouteProgress &&
         left.firstResetLastGroundedPosition.x == right.firstResetLastGroundedPosition.x &&
         left.firstResetLastGroundedPosition.y == right.firstResetLastGroundedPosition.y &&
         left.firstResetLastGroundedPosition.z == right.firstResetLastGroundedPosition.z &&
         left.firstResetOrientation == right.firstResetOrientation &&
         left.firstResetAngularVelocity.x == right.firstResetAngularVelocity.x &&
         left.firstResetAngularVelocity.y == right.firstResetAngularVelocity.y &&
         left.firstResetAngularVelocity.z == right.firstResetAngularVelocity.z &&
         left.firstResetSpeed == right.firstResetSpeed &&
         left.error == right.error;
}

} // namespace

int main(const int argc, const char* const* argv) {
  int triggerArgument = argc;
  int driveArgument = argc;
  int routeArgument = argc;
  for (int argument = 5; argument < argc; ++argument) {
    if (std::string_view(argv[argument]) == "--trigger") {
      triggerArgument = argument;
    } else if (std::string_view(argv[argument]) == "--drive") {
      driveArgument = argument;
    } else if (std::string_view(argv[argument]) == "--route") {
      routeArgument = argument;
    }
  }
  const int groundEnd = std::min({triggerArgument, driveArgument, routeArgument});
  const int triggerEnd = std::min(driveArgument, routeArgument);
  const int driveEnd = routeArgument;
  if (argc < 5 ||
      (triggerArgument != argc && triggerArgument > triggerEnd) ||
      (driveArgument != argc && routeArgument != argc && driveArgument > routeArgument) ||
      (driveArgument != argc && routeArgument != argc) ||
      (groundEnd - 5) % 3 != 0 ||
      (triggerArgument != argc && (triggerEnd - triggerArgument - 1) % 6 != 0) ||
      (driveArgument != argc &&
       ((driveEnd - driveArgument - 1) == 0 || (driveEnd - driveArgument - 1) % 4 != 0)) ||
      (routeArgument != argc &&
       ((argc - routeArgument - 1) == 0 || (argc - routeArgument - 1) % 3 != 0))) {
    std::cerr << "usage: archive_track_scene <Stadium.pak> <packlist.dat> <Challenge.Gbx> "
                 "<GameData-root> [ground-x ground-y ground-z ...] "
                 "[--trigger from-x from-y from-z to-x to-y to-z ...] "
                 "[--drive time-ms steering throttle brake ...] "
                 "[--route x y z ...]\n";
    return 2;
  }
  const auto parseCoordinate = [](const char* text, float& value) {
    char* end = nullptr;
    errno = 0;
    value = std::strtof(text, &end);
    return errno == 0 && end != text && *end == '\0' && std::isfinite(value);
  };
  std::vector<tmnf::gbx::Float3> groundQueries;
  for (int argument = 5; argument < groundEnd; argument += 3) {
    tmnf::gbx::Float3 query;
    float* const values[] = {&query.x, &query.y, &query.z};
    for (int component = 0; component < 3; ++component) {
      if (!parseCoordinate(argv[argument + component], *values[component])) {
        std::cerr << "invalid ground query coordinate\n";
        return 2;
      }
    }
    groundQueries.push_back(query);
  }
  std::vector<std::pair<tmnf::gbx::Float3, tmnf::gbx::Float3>> triggerQueries;
  for (int argument = triggerArgument + 1; argument < triggerEnd; argument += 6) {
    tmnf::gbx::Float3 from;
    tmnf::gbx::Float3 to;
    float* const values[] = {&from.x, &from.y, &from.z, &to.x, &to.y, &to.z};
    for (int component = 0; component < 6; ++component) {
      if (!parseCoordinate(argv[argument + component], *values[component])) {
        std::cerr << "invalid trigger query coordinate\n";
        return 2;
      }
    }
    triggerQueries.emplace_back(from, to);
  }
  std::vector<DriveEvent> driveEvents;
  for (int argument = driveArgument + 1; argument < driveEnd; argument += 4) {
    float timeMilliseconds = 0.0F;
    DriveEvent event;
    if (!parseCoordinate(argv[argument], timeMilliseconds) || timeMilliseconds < 0.0F ||
        !parseCoordinate(argv[argument + 1], event.control.steering) ||
        !parseCoordinate(argv[argument + 2], event.control.throttle) ||
        !parseCoordinate(argv[argument + 3], event.control.brake) ||
        std::abs(event.control.steering) > 1.0F || event.control.throttle < 0.0F ||
        event.control.throttle > 1.0F || event.control.brake < 0.0F ||
        event.control.brake > 1.0F ||
        (!driveEvents.empty() && timeMilliseconds < driveEvents.back().timeSeconds * 1000.0)) {
      std::cerr << "invalid control drive event\n";
      return 2;
    }
    event.timeSeconds = static_cast<double>(timeMilliseconds) / 1000.0;
    driveEvents.push_back(event);
  }
  std::vector<tmnf::gbx::Float3> route;
  for (int argument = routeArgument + 1; argument < argc; argument += 3) {
    tmnf::gbx::Float3 point;
    float* const values[] = {&point.x, &point.y, &point.z};
    for (int component = 0; component < 3; ++component) {
      if (!parseCoordinate(argv[argument + component], *values[component])) {
        std::cerr << "invalid control route coordinate\n";
        return 2;
      }
    }
    route.push_back(point);
  }
  std::vector<std::byte> archiveBytes;
  std::vector<std::byte> packListBytes;
  std::vector<std::byte> challengeBytes;
  if (!readFile(argv[1], archiveBytes) || !readFile(argv[2], packListBytes) ||
      !readFile(argv[3], challengeBytes)) {
    std::cerr << "unable to read archive, packlist, or challenge\n";
    return 2;
  }
  tmnf::pak::VirtualFileSystem vfs;
  const tmnf::pak::VfsOperationResult initialized = vfs.initialize(packListBytes);
  if (!initialized) {
    std::cerr << initialized.error << '\n';
    return 1;
  }
  const tmnf::pak::VfsOperationResult mounted = vfs.mount("Stadium", std::move(archiveBytes));
  if (!mounted) {
    std::cerr << mounted.error << '\n';
    return 1;
  }
  tmnf::pak::VfsOpenResult tuningInput =
      vfs.openFile("Vehicles\\VehicleTunings\\StadiumCar.VehicleTunings.Gbx");
  if (!tuningInput) {
    std::cerr << tuningInput.error << '\n';
    return 1;
  }
  tmnf::gbx::VehicleTuningsParseResult parsedTunings =
      tmnf::gbx::parseVehicleTunings(*tuningInput.input);
  if (!parsedTunings || parsedTunings.tunings.active() == nullptr) {
    std::cerr << (parsedTunings.error.empty() ? "vehicle tuning has no active entry"
                                                : parsedTunings.error)
              << '\n';
    return 1;
  }
  const tmnf::gbx::VehicleTuning& activeTuning = *parsedTunings.tunings.active();
  tmnf::pak::VfsOpenResult vehicleSolidInput =
      vfs.openFile("Vehicles\\Media\\Solid\\StadiumCar.Solid.Gbx");
  if (!vehicleSolidInput) {
    std::cerr << vehicleSolidInput.error << '\n';
    return 1;
  }
  tmnf::gbx::SolidParseResult parsedVehicleSolid =
      tmnf::gbx::parseSolid(*vehicleSolidInput.input);
  if (!parsedVehicleSolid) {
    std::cerr << parsedVehicleSolid.error << '\n';
    return 1;
  }
  tmnf::pak::VfsOpenResult soundEngineInput =
      vfs.openFile("Vehicles\\Media\\Audio\\Sound\\StadiumCarEngine.SoundEngine.Gbx");
  if (!soundEngineInput) {
    std::cerr << soundEngineInput.error << '\n';
    return 1;
  }
  tmnf::gbx::SoundEngineParseResult parsedSoundEngine =
      tmnf::gbx::parseSoundEngine(*soundEngineInput.input);
  if (!parsedSoundEngine) {
    std::cerr << parsedSoundEngine.error << '\n';
    return 1;
  }
  tmnf::gbx::ChallengeParseResult parsed = tmnf::gbx::parseChallenge(challengeBytes);
  if (!parsed) {
    std::cerr << parsed.error << '\n';
    return 1;
  }
  const std::filesystem::path gameDataRoot(argv[4]);
  const tmnf::content::LooseFileReader looseReader =
      [&gameDataRoot](const std::string_view path) -> std::optional<std::vector<std::byte>> {
    std::vector<std::byte> bytes;
    if (!readFile(gameDataRoot / nativePath(path), bytes)) {
      return std::nullopt;
    }
    return bytes;
  };
  tmnf::content::TrackSceneBuildResult built =
      tmnf::content::buildTrackScene(std::move(parsed.challenge), vfs, looseReader);
  if (!built) {
    std::cerr << built.error << '\n';
    return 1;
  }
  const tmnf::content::TrackScene& scene = built.scene;
  const tmnf::content::TrackCollisionBuildResult collisionBuilt =
      tmnf::content::buildTrackCollision(scene);
  if (!collisionBuilt) {
    std::cerr << collisionBuilt.error << '\n';
    return 1;
  }
  const tmnf::content::TrackCollision& collision = collisionBuilt.collision;
  tmnf::gameplay::VehicleSimulation vehicle;
  const tmnf::gameplay::VehicleInitializeResult vehicleInitialized =
      vehicle.initialize(scene, collision, activeTuning, &parsedVehicleSolid.solid);
  if (!vehicleInitialized) {
    std::cerr << vehicleInitialized.error << '\n';
    return 1;
  }
  const tmnf::gameplay::VehicleState& vehicleSpawn = vehicle.state();
  std::size_t occupiedCollisionCells = 0;
  for (const auto& cell : collision.cellTriangles) {
    occupiedCollisionCells += cell.empty() ? 0U : 1U;
  }
  std::array<std::size_t, 31> collisionSurfaceTriangleCounts{};
  for (const std::int16_t surfaceId : collision.triangleSurfaceIds) {
    if (surfaceId < 0 || static_cast<std::size_t>(surfaceId) >=
                             collisionSurfaceTriangleCounts.size()) {
      std::cerr << "track collision contains an unresolved surface material\n";
      return 1;
    }
    ++collisionSurfaceTriangleCounts[static_cast<std::size_t>(surfaceId)];
  }
  const tmnf::gbx::ChallengeBlock* startBlock = nullptr;
  for (const tmnf::gbx::ChallengeBlock& block : scene.challenge.blocks) {
    if (block.name == "StadiumRoadMainStartLine") {
      startBlock = &block;
      break;
    }
  }
  std::optional<tmnf::content::TrackGroundHit> startGround;
  tmnf::gbx::Float3 startCenter;
  std::array<std::size_t, 4> startGroundSamples{};
  if (startBlock != nullptr) {
    startCenter = vehicleSpawn.position;
    startGround = tmnf::content::sampleTrackGround(collision, startCenter.x, startCenter.z,
                                                   startCenter.y - 16.0F, startCenter.y + 32.0F);
    constexpr std::array<tmnf::gbx::Float3, 4> directions{
        tmnf::gbx::Float3{1.0F, 0.0F, 0.0F}, tmnf::gbx::Float3{-1.0F, 0.0F, 0.0F},
        tmnf::gbx::Float3{0.0F, 0.0F, 1.0F}, tmnf::gbx::Float3{0.0F, 0.0F, -1.0F}};
    for (std::size_t direction = 0; direction < directions.size(); ++direction) {
      for (float distance = 4.0F; distance <= 64.0F; distance += 4.0F) {
        const auto hit = tmnf::content::sampleTrackGround(
            collision, startCenter.x + directions[direction].x * distance,
            startCenter.z + directions[direction].z * distance, startCenter.y - 4.0F,
            startCenter.y + 4.0F);
        startGroundSamples[direction] += hit.has_value() ? 1U : 0U;
      }
    }
  }
  std::size_t groundQueryHits = 0;
  float groundQueryMaximumDelta = 0.0F;
  std::string groundQueryMask;
  groundQueryMask.reserve(groundQueries.size());
  std::optional<std::size_t> firstGroundQueryMiss;
  std::optional<std::size_t> firstGroundQueryMismatch;
  for (std::size_t index = 0; index < groundQueries.size(); ++index) {
    const tmnf::gbx::Float3 query = groundQueries[index];
    const auto hit = tmnf::content::sampleTrackGround(collision, query.x, query.z, query.y - 2.0F,
                                                      query.y + 0.5F);
    if (!hit.has_value()) {
      groundQueryMask.push_back('0');
      if (!firstGroundQueryMiss.has_value()) {
        firstGroundQueryMiss = index;
      }
      continue;
    }
    ++groundQueryHits;
    const float delta = std::abs(hit->height - query.y);
    groundQueryMask.push_back(delta <= 0.5F ? '1' : '2');
    groundQueryMaximumDelta = std::max(groundQueryMaximumDelta, delta);
    if (delta > 0.5F && !firstGroundQueryMismatch.has_value()) {
      firstGroundQueryMismatch = index;
    }
  }
  std::size_t triggerQueryHits = 0;
  std::string triggerQueryMask;
  triggerQueryMask.reserve(triggerQueries.size());
  for (const auto& [from, to] : triggerQueries) {
    const bool hit = std::ranges::any_of(collision.triggers, [&](const auto& trigger) {
      return tmnf::content::segmentIntersectsTrackTrigger(trigger, from, to);
    });
    triggerQueryHits += hit ? 1U : 0U;
    triggerQueryMask.push_back(hit ? '1' : '0');
  }
  std::optional<DriveResult> controlDrive;
  std::optional<tmnf::content::TrackWallContact> controlDriveFinalWall;
  bool controlDriveDeterministic = false;
  if (!driveEvents.empty() || !route.empty()) {
    controlDrive = route.empty()
                       ? runDrive(scene, collision, activeTuning, parsedVehicleSolid.solid,
                                  driveEvents)
                       : runRoute(scene, collision, activeTuning, parsedVehicleSolid.solid,
                                  route);
    const DriveResult repeated =
        route.empty()
            ? runDrive(scene, collision, activeTuning, parsedVehicleSolid.solid, driveEvents)
            : runRoute(scene, collision, activeTuning, parsedVehicleSolid.solid, route);
    controlDriveDeterministic = sameDrive(*controlDrive, repeated);
    controlDriveFinalWall = tmnf::content::sampleTrackWallContact(
        collision,
        {controlDrive->state.position.x, controlDrive->state.position.y + 0.45F,
         controlDrive->state.position.z},
        0.91F);
  }
  std::vector<std::size_t> modelPlacementCounts(scene.models.size());
  for (const tmnf::content::TrackPlacement& placement : scene.placements) {
    ++modelPlacementCounts[placement.modelIndex];
  }
  std::size_t uniqueVisuals = 0;
  std::size_t uniqueVertices = 0;
  std::size_t uniqueIndices = 0;
  std::size_t instancedVertices = 0;
  std::size_t instancedIndices = 0;
  std::size_t triggerVertices = 0;
  std::size_t triggerIndices = 0;
  for (const tmnf::content::TrackTriggerMesh& trigger : collision.triggers) {
    triggerVertices += trigger.positions.size();
    triggerIndices += trigger.indices.size();
  }
  for (std::size_t modelIndex = 0; modelIndex < scene.models.size(); ++modelIndex) {
    const tmnf::content::TrackModel& model = scene.models[modelIndex];
    for (const tmnf::gbx::SolidVisual& visual : model.solid.visuals) {
      if (!visual.renderEnabled) {
        continue;
      }
      ++uniqueVisuals;
      uniqueVertices += visual.positions.size();
      uniqueIndices += visual.indices.size();
      instancedVertices += visual.positions.size() * modelPlacementCounts[modelIndex];
      instancedIndices += visual.indices.size() * modelPlacementCounts[modelIndex];
    }
  }

  std::cout << "{\n"
            << "  \"mapName\": \"" << jsonEscape(scene.challenge.mapName) << "\",\n"
            << "  \"challengeBlockCount\": " << scene.challenge.blocks.size() << ",\n"
            << "  \"placementCount\": " << scene.placements.size() << ",\n"
            << "  \"invisibleBlockCount\": " << scene.invisibleBlockCount << ",\n"
            << "  \"modelCount\": " << scene.models.size() << ",\n"
            << "  \"textureCount\": " << scene.textures.size() << ",\n"
            << "  \"uniqueEnabledVisualCount\": " << uniqueVisuals << ",\n"
            << "  \"uniqueVertexCount\": " << uniqueVertices << ",\n"
            << "  \"uniqueIndexCount\": " << uniqueIndices << ",\n"
            << "  \"instancedVertexCount\": " << instancedVertices << ",\n"
            << "  \"instancedIndexCount\": " << instancedIndices << ",\n"
            << "  \"collisionVertexCount\": " << collision.positions.size() << ",\n"
            << "  \"collisionIndexCount\": " << collision.indices.size() << ",\n"
            << "  \"collisionSurfaceTriangleCounts\": [";
  for (std::size_t index = 0; index < collisionSurfaceTriangleCounts.size(); ++index) {
    std::cout << collisionSurfaceTriangleCounts[index]
              << (index + 1U == collisionSurfaceTriangleCounts.size() ? "" : ",");
  }
  std::cout << "],\n"
            << "  \"triggerCount\": " << collision.triggers.size() << ",\n"
            << "  \"triggerVertexCount\": " << triggerVertices << ",\n"
            << "  \"triggerIndexCount\": " << triggerIndices << ",\n"
            << "  \"orderedCheckpointCount\": " << scene.challenge.checkpoints.size() << ",\n"
            << "  \"vehicleCollisionShapes\": [";
  for (std::size_t index = 0; index < parsedVehicleSolid.solid.collisionShapes.size(); ++index) {
    const tmnf::gbx::SolidCollisionShape& shape =
        parsedVehicleSolid.solid.collisionShapes[index];
    std::cout << "{\"name\":\"" << jsonEscape(shape.treeName) << "\",\"surfaceId\":"
              << shape.surfaceId << ",\"halfAxes\":[" << shape.maximum.x << ','
              << shape.maximum.y << ',' << shape.maximum.z << "],\"transform\":[";
    for (std::size_t component = 0; component < shape.transform.size(); ++component) {
      std::cout << shape.transform[component]
                << (component + 1U == shape.transform.size() ? "" : ",");
    }
    std::cout << "]}" << (index + 1U == parsedVehicleSolid.solid.collisionShapes.size()
                                ? ""
                                : ",");
  }
  std::cout << "],\n"
            << "  \"soundEngine\": {\"volume\": " << parsedSoundEngine.engine.volume
            << ", \"maxRpm\": " << parsedSoundEngine.engine.maxRpm
            << ", \"components\": [";
  for (std::size_t index = 0; index < parsedSoundEngine.engine.components.size(); ++index) {
    const tmnf::gbx::SoundEngineComponent& component =
        parsedSoundEngine.engine.components[index];
    std::cout << "{\"path\":\"" << jsonEscape(component.soundPath)
              << "\",\"minVolume\":" << component.minVolume
              << ",\"maxVolume\":" << component.maxVolume
              << ",\"fadeInStartRpm\":" << component.fadeInStartRpm
              << ",\"fadeInEndRpm\":" << component.fadeInEndRpm
              << ",\"fadeOutStartRpm\":" << component.fadeOutStartRpm
              << ",\"fadeOutEndRpm\":" << component.fadeOutEndRpm
              << ",\"minPitch\":" << component.minPitch
              << ",\"maxPitch\":" << component.maxPitch
              << ",\"pitchShiftStartRpm\":" << component.pitchShiftStartRpm
              << ",\"pitchShiftEndRpm\":" << component.pitchShiftEndRpm << '}';
    if (index + 1U != parsedSoundEngine.engine.components.size()) {
      std::cout << ',';
    }
  }
  std::cout << "], \"volumeCurves\":";
  writeCurves(parsedSoundEngine.engine.volumeCurves);
  std::cout << ", \"alphaCurves\":";
  writeCurves(parsedSoundEngine.engine.alphaCurves);
  std::cout << "},\n"
            << "  \"vehicleTuning\": {\"count\": " << parsedTunings.tunings.entries.size()
            << ", \"activeIndex\": " << parsedTunings.tunings.activeIndex << ", \"name\": \""
            << jsonEscape(activeTuning.name) << "\", \"mass\": " << activeTuning.mass
            << ", \"maxSpeed\": " << activeTuning.maxSpeed
            << ", \"reverseMaxSpeed\": " << activeTuning.reverseMaxSpeed
            << ", \"steerSpeed\": " << activeTuning.steerSpeed
            << ", \"steerModel\": " << activeTuning.steerModel
            << ", \"steerGroundTorque\": " << activeTuning.steerGroundTorque
            << ", \"suspensionSpring\": " << activeTuning.suspensionSpring
            << ", \"suspensionDamping\": " << activeTuning.suspensionDamping
            << ", \"suspensionMinimum\": " << activeTuning.suspensionMinimum
            << ", \"suspensionMaximum\": " << activeTuning.suspensionMaximum
            << ", \"suspensionRest\": " << activeTuning.suspensionRest
            << ", \"relativeSpeedMultiplier\": " << activeTuning.relativeSpeedMultiplier
             << ", \"absorbTension\": " << activeTuning.absorbTension
             << ", \"turboBoost\": " << activeTuning.turboBoost
             << ", \"turbo2Boost\": " << activeTuning.turbo2Boost
             << ", \"turboDuration\": " << activeTuning.turboDuration
             << ", \"turbo2Duration\": " << activeTuning.turbo2Duration
            << ", \"centerOfMassAftForce\": " << activeTuning.centerOfMassAftForce
            << ", \"centerOfMassDownUp\": " << activeTuning.centerOfMassDownUp
            << ", \"inertiaMass\": " << activeTuning.inertiaMass
            << ", \"inertiaHalfDiagonal\": [" << activeTuning.inertiaHalfDiagonal[0] << ','
            << activeTuning.inertiaHalfDiagonal[1] << ','
            << activeTuning.inertiaHalfDiagonal[2] << ']'
            << ", \"tireMaterial\": " << activeTuning.tireMaterial
            << ", \"shockModel\": " << activeTuning.shockModel
            << ", \"gravityCoef\": " << activeTuning.gravityCoef
            << ", \"gravityCoefAir\": " << activeTuning.gravityCoefAir
            << ", \"linearFluidFrictionCoef\": " << activeTuning.linearFluidFrictionCoef
            << ", \"angularFluidFrictionCoef1\": "
            << activeTuning.angularFluidFrictionCoef1
            << ", \"angularFluidFrictionCoef2\": "
            << activeTuning.angularFluidFrictionCoef2
            << ", \"airControlDuration\": " << activeTuning.airControlDuration
            << ", \"maxAngularSpeedYAirControl\": "
            << activeTuning.maxAngularSpeedYAirControl
            << ", \"bodyFrictionCoefConcrete\": "
            << activeTuning.bodyFrictionCoefConcrete
            << ", \"bodyFrictionCoefMetal\": " << activeTuning.bodyFrictionCoefMetal
            << ", \"bodyRestCoefMetal\": " << activeTuning.bodyRestCoefMetal
            << ", \"bodyRestCoefConcrete\": " << activeTuning.bodyRestCoefConcrete
            << ", \"wheelFrictionCoefConcrete\": "
            << activeTuning.wheelFrictionCoefConcrete
            << ", \"wheelRestCoefConcrete\": " << activeTuning.wheelRestCoefConcrete
            << ", \"wheelFrictionCoefMetal\": "
            << activeTuning.wheelFrictionCoefMetal
            << ", \"wheelRestCoefMetal\": " << activeTuning.wheelRestCoefMetal
            << ", \"angularSpeedYImpulseScale\": "
            << activeTuning.angularSpeedYImpulseScale
            << ", \"angularSpeedClamp\": " << activeTuning.angularSpeedClamp
            << ", \"linearSpeed2PositiveDeltaMax\": "
            << activeTuning.linearSpeed2PositiveDeltaMax
            << ", \"steerLowSpeed\": " << activeTuning.steerLowSpeed
            << ", \"steerAngleMax\": " << activeTuning.steerAngleMax
            << ", \"slipAngleForceMax\": " << activeTuning.slipAngleForceMax
            << ", \"slipAngleForceCoef1\": " << activeTuning.slipAngleForceCoef1
            << ", \"slipAngleForceCoef2\": " << activeTuning.slipAngleForceCoef2
            << ", \"axialSlopeAdherenceMin\": "
            << activeTuning.axialSlopeAdherenceMin
            << ", \"axialSlopeAdherenceMax\": "
            << activeTuning.axialSlopeAdherenceMax
            << ", \"steerGroundTorqueSlippingCoef\": "
            << activeTuning.steerGroundTorqueSlippingCoef
            << ", \"maxSideFrictionBlendCoef\": "
            << activeTuning.maxSideFrictionBlendCoef
            << ", \"maxSideFriction\": ";
  writeCurves(std::array{activeTuning.maxSideFriction});
  std::cout << ", \"maxSideFrictionSliding\": " << activeTuning.maxSideFrictionSliding
            << ", \"sideFriction1\": " << activeTuning.sideFriction1
            << ", \"lateralSlopeAdherenceMin\": "
            << activeTuning.lateralSlopeAdherenceMin
            << ", \"lateralSlopeAdherenceMax\": "
            << activeTuning.lateralSlopeAdherenceMax
            << ", \"rolloverLateral\": ";
  writeCurves(std::array{activeTuning.rolloverLateral});
  std::cout << ", \"steerSlowDown\": ";
  writeCurves(std::array{activeTuning.steerSlowDown});
  std::cout << ", \"steerSlowDownCoef\": " << activeTuning.steerSlowDownCoef;
  std::cout << ", \"visualSteerAngleFromSpeed\": ";
  writeCurves(std::array{activeTuning.visualSteerAngleFromSpeed});
  std::cout << ", \"airControlZCoefFromAngularSpeed\": ";
  writeCurves(std::array{activeTuning.airControlZCoefFromAngularSpeed});
  std::cout << ", \"steerDriveTorque\": ";
  writeCurves(std::array{activeTuning.steerDriveTorque});
  std::cout
            << ", \"modulationFromWheelCompression\": ";
  writeCurves(std::array{activeTuning.modulationFromWheelCompression});
  std::cout << ", \"frictionModulationWhenSlipAndBrake\": "
            << activeTuning.m6FrictionModulationWhenSlipAndBrake
            << ", \"m6InertialMass\": " << activeTuning.m6InertialMass
            << ", \"m6ForceEpsilon\": " << activeTuning.m6ForceEpsilon
            << ", \"m6InertialTorqueModulationX\": "
            << activeTuning.m6InertialTorqueModulationX
            << ", \"m6InertialTorqueModulationZ\": "
            << activeTuning.m6InertialTorqueModulationZ
            << ", \"m6RolloverLateralFromSpeedRatio\": ";
  writeCurves(std::array{activeTuning.m6RolloverLateralFromSpeedRatio});
  std::cout << ", \"m6BurnoutRolloverFromSpeed\": ";
  writeCurves(std::array{activeTuning.m6BurnoutRolloverFromSpeed});
  std::cout << ", \"m6DonutRolloverFromSpeed\": ";
  writeCurves(std::array{activeTuning.m6DonutRolloverFromSpeed});
  std::cout << ", \"fakeEngine\": " << (activeTuning.fakeEngine ? "true" : "false")
            << ", \"m6MaxRpm\": " << activeTuning.m6MaxRpm << ", \"gearRatio\": ";
  writeFloatArray(activeTuning.m6GearRatio);
  std::cout << ", \"maxRpmByGear\": ";
  writeFloatArray(activeTuning.m6MaxRpmByGear);
  std::cout << ", \"minRpmByGear\": ";
  writeFloatArray(activeTuning.m6MinRpmByGear);
  std::cout << ", \"rpmWantedOnGearUp\": ";
  writeFloatArray(activeTuning.m6RpmWantedOnGearUp);
  std::cout << ", \"burnoutRpmAcceleration\": " << activeTuning.m6BurnoutRpmAcceleration
            << ", \"airRpmAcceleration\": " << activeTuning.m6AirRpmAcceleration
            << ", \"airRpmDeadening\": " << activeTuning.m6AirRpmDeadening
            << ", \"rpmLossOnGearUp\": " << activeTuning.m6RpmLossOnGearUp
            << ", \"rpmGainOnGearDown\": " << activeTuning.m6RpmGainOnGearDown
            << ", \"speedLimitPositiveForTakeOffFront\": "
            << activeTuning.m6SpeedLimitPositiveForTakeOffFront
            << ", \"speedLimitPositiveForTakeOffRear\": "
            << activeTuning.m6SpeedLimitPositiveForTakeOffRear
            << ", \"speedLimitNegativeForTakeOffFront\": "
            << activeTuning.m6SpeedLimitNegativeForTakeOffFront
            << ", \"speedLimitNegativeForTakeOffRear\": "
            << activeTuning.m6SpeedLimitNegativeForTakeOffRear
            << ", \"rpmGainOnTakeOff\": " << activeTuning.m6RpmGainOnTakeOff
            << ", \"rpmLossOnTakeOffFinished\": "
            << activeTuning.m6RpmLossOnTakeOffFinished;
  std::cout << "},\n"
            << "  \"vehicleCheckpointCount\": " << vehicle.checkpointTotal() << ",\n"
            << "  \"occupiedCollisionCellCount\": " << occupiedCollisionCells << ",\n"
            << "  \"vehicleSpawn\": {\"x\": " << vehicleSpawn.position.x
            << ", \"y\": " << vehicleSpawn.position.y << ", \"z\": " << vehicleSpawn.position.z
            << ", \"heading\": " << vehicleSpawn.heading << "},\n"
            << "  \"startGround\": {\"x\": " << startCenter.x << ", \"z\": " << startCenter.z
            << ", \"height\": " << (startGround.has_value() ? startGround->height : -1.0F)
            << ", \"surfaceId\": " << (startGround.has_value() ? startGround->surfaceId : -1)
            << "},\n"
            << "  \"startGroundSamples\": {\"positiveX\": " << startGroundSamples[0]
            << ", \"negativeX\": " << startGroundSamples[1]
            << ", \"positiveZ\": " << startGroundSamples[2]
            << ", \"negativeZ\": " << startGroundSamples[3] << "},\n"
            << "  \"groundQueries\": {\"count\": " << groundQueries.size()
            << ", \"hits\": " << groundQueryHits
            << ", \"mask\": \"" << groundQueryMask << "\""
            << ", \"maximumDelta\": " << groundQueryMaximumDelta
            << ", \"firstMiss\": "
            << (firstGroundQueryMiss.has_value() ? std::to_string(*firstGroundQueryMiss) : "null")
            << ", \"firstMismatch\": "
            << (firstGroundQueryMismatch.has_value() ? std::to_string(*firstGroundQueryMismatch)
                                                     : "null")
            << "},\n"
            << "  \"triggerQueries\": {\"count\": " << triggerQueries.size()
            << ", \"hits\": " << triggerQueryHits << ", \"mask\": \""
            << triggerQueryMask << "\"},\n";
  if (!controlDrive.has_value()) {
    std::cout << "  \"controlDrive\": null,\n";
  } else {
    std::cout << "  \"controlDrive\": {\"mode\": \""
              << (route.empty() ? "events" : "route") << "\", \"eventCount\": "
              << driveEvents.size() << ", \"routePointCount\": " << route.size()
              << ", \"routeProgress\": " << controlDrive->routeProgress
              << ", \"checkpointSequence\": [";
    for (std::size_t index = 0; index < controlDrive->checkpointSequence.size(); ++index) {
      std::cout << controlDrive->checkpointSequence[index]
                << (index + 1U == controlDrive->checkpointSequence.size() ? "" : ",");
    }
    std::cout << "], \"checkpointTimes\": ";
    writeFloatArray(controlDrive->checkpointTimes);
    std::cout << ", \"checkpointPositions\": ";
    writeFloatArray(controlDrive->checkpointPositions);
    std::cout << ", \"checkpointRouteProgress\": [";
    for (std::size_t index = 0; index < controlDrive->checkpointRouteProgress.size(); ++index) {
      std::cout << controlDrive->checkpointRouteProgress[index]
                << (index + 1U == controlDrive->checkpointRouteProgress.size() ? "" : ",");
    }
    std::cout << ']';
    std::cout << ", \"checkpointCount\": " << controlDrive->state.checkpointCount
              << ", \"finished\": " << (controlDrive->state.finished ? "true" : "false")
              << ", \"raceTime\": " << controlDrive->state.raceTimeSeconds
              << ", \"completedLaps\": " << controlDrive->state.completedLaps
              << ", \"lapTotal\": " << vehicle.lapTotal()
              << ", \"grounded\": " << (controlDrive->state.grounded ? "true" : "false")
              << ", \"finite\": " << (controlDrive->finite ? "true" : "false")
              << ", \"deterministic\": " << (controlDriveDeterministic ? "true" : "false")
              << ", \"simulatedSeconds\": " << controlDrive->simulatedSeconds
              << ", \"maxStepDistance\": " << controlDrive->maxStepDistance
              << ", \"firstAirSeconds\": " << controlDrive->firstAirSeconds
              << ", \"firstAirRouteProgress\": " << controlDrive->firstAirRouteProgress
               << ", \"firstAirPosition\": [" << controlDrive->firstAirPosition.x << ','
               << controlDrive->firstAirPosition.y << ',' << controlDrive->firstAirPosition.z << ']'
               << ", \"firstAirVelocity\": [" << controlDrive->firstAirVelocity.x << ','
               << controlDrive->firstAirVelocity.y << ',' << controlDrive->firstAirVelocity.z << ']'
               << ", \"firstAirAngularVelocity\": ["
               << controlDrive->firstAirAngularVelocity.x << ','
               << controlDrive->firstAirAngularVelocity.y << ','
               << controlDrive->firstAirAngularVelocity.z << ']'
               << ", \"firstAirOrientation\": [" << controlDrive->firstAirOrientation[0] << ','
               << controlDrive->firstAirOrientation[1] << ','
               << controlDrive->firstAirOrientation[2] << ','
               << controlDrive->firstAirOrientation[3] << ']'
               << ", \"firstAirHeading\": " << controlDrive->firstAirHeading
               << ", \"firstAirSpeed\": " << controlDrive->firstAirSpeed
               << ", \"airTransitions\": ";
    writeFloatArray(controlDrive->airTransitions);
    std::cout << ", \"routeMilestones\": ";
    writeFloatArray(controlDrive->routeMilestones);
    std::cout
               << ", \"resetCount\": " << controlDrive->resetCount
              << ", \"firstResetSeconds\": " << controlDrive->firstResetSeconds
              << ", \"firstResetRouteProgress\": "
              << controlDrive->firstResetRouteProgress
              << ", \"firstResetFrom\": [" << controlDrive->firstResetFrom.x << ','
              << controlDrive->firstResetFrom.y << ',' << controlDrive->firstResetFrom.z << ']'
              << ", \"firstResetTo\": [" << controlDrive->firstResetTo.x << ','
              << controlDrive->firstResetTo.y << ',' << controlDrive->firstResetTo.z << ']'
              << ", \"firstResetLastGroundedSeconds\": "
              << controlDrive->firstResetLastGroundedSeconds
              << ", \"firstResetLastGroundedRouteProgress\": "
              << controlDrive->firstResetLastGroundedRouteProgress
              << ", \"firstResetLastGroundedPosition\": ["
              << controlDrive->firstResetLastGroundedPosition.x << ','
              << controlDrive->firstResetLastGroundedPosition.y << ','
              << controlDrive->firstResetLastGroundedPosition.z << ']'
              << ", \"firstResetOrientation\": [" << controlDrive->firstResetOrientation[0]
              << ',' << controlDrive->firstResetOrientation[1] << ','
              << controlDrive->firstResetOrientation[2] << ','
              << controlDrive->firstResetOrientation[3] << ']'
              << ", \"firstResetAngularVelocity\": ["
              << controlDrive->firstResetAngularVelocity.x << ','
              << controlDrive->firstResetAngularVelocity.y << ','
              << controlDrive->firstResetAngularVelocity.z << ']'
              << ", \"firstResetSpeed\": " << controlDrive->firstResetSpeed
              << ", \"finalPosition\": [" << controlDrive->state.position.x << ','
              << controlDrive->state.position.y << ',' << controlDrive->state.position.z << ']'
              << ", \"finalHeading\": " << controlDrive->state.heading
              << ", \"finalSpeed\": " << controlDrive->state.speed
              << ", \"finalGear\": " << controlDrive->state.gear
              << ", \"finalEngineRpm\": " << controlDrive->state.engineRpm
              << ", \"finalEngineTargetRpm\": " << controlDrive->state.engineTargetRpm
              << ", \"finalGearShiftSeconds\": " << controlDrive->state.gearShiftSeconds
              << ", \"finalWallTriangle\": "
              << (controlDriveFinalWall.has_value()
                      ? std::to_string(controlDriveFinalWall->triangleIndex)
                      : "null")
              << ", \"finalWallSurface\": "
              << (controlDriveFinalWall.has_value()
                      ? std::to_string(controlDriveFinalWall->surfaceId)
                      : "null")
              << ", \"finalWallVertices\": [";
    if (controlDriveFinalWall.has_value()) {
      const std::size_t first =
          static_cast<std::size_t>(controlDriveFinalWall->triangleIndex) * 3U;
      for (std::size_t corner = 0; corner < 3U; ++corner) {
        const tmnf::gbx::Float3 point = collision.positions[collision.indices[first + corner]];
        std::cout << '[' << point.x << ',' << point.y << ',' << point.z << ']'
                  << (corner == 2U ? "" : ",");
      }
    }
    std::cout << "], \"error\": \"" << jsonEscape(controlDrive->error) << "\"},\n";
  }
  std::cout << "  \"models\": [\n";
  for (std::size_t modelIndex = 0; modelIndex < scene.models.size(); ++modelIndex) {
    const tmnf::content::TrackModel& model = scene.models[modelIndex];
    std::size_t enabled = 0;
    for (const tmnf::gbx::SolidVisual& visual : model.solid.visuals) {
      enabled += visual.renderEnabled ? 1U : 0U;
    }
    std::cout << "    {\"path\": \"" << jsonEscape(model.logicalPath)
              << "\", \"placements\": " << modelPlacementCounts[modelIndex]
              << ", \"visuals\": " << model.solid.visuals.size()
              << ", \"enabledVisuals\": " << enabled << "}"
              << (modelIndex + 1U == scene.models.size() ? "" : ",") << '\n';
  }
  std::cout << "  ],\n  \"textures\": [\n";
  for (std::size_t index = 0; index < scene.textures.size(); ++index) {
    const tmnf::content::ResolvedMaterialTexture& texture = scene.textures[index];
    std::cout << "    {\"image\": \"" << jsonEscape(texture.imagePath)
              << "\", \"width\": " << texture.image.width
              << ", \"height\": " << texture.image.height << "}"
              << (index + 1U == scene.textures.size() ? "" : ",") << '\n';
  }
  std::cout << "  ]\n}\n";
  return 0;
}
