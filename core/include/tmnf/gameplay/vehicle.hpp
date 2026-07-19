#pragma once

#include "tmnf/content/track_collision.hpp"
#include "tmnf/content/track_scene.hpp"
#include "tmnf/gbx/solid.hpp"
#include "tmnf/gbx/vehicle_tunings.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace tmnf::gameplay {

struct VehicleControl final {
  float steering = 0.0F;
  float throttle = 0.0F;
  float brake = 0.0F;
};

struct VehicleWheelState final {
  float suspensionLength = 0.0F;
  float suspensionSpeed = 0.0F;
  float pendingCorrection = 0.0F;
  gbx::Float3 contactNormal{0.0F, 1.0F, 0.0F};
  gbx::Float3 contactPoint;
  bool contact = false;
  bool slipping = false;

  bool operator==(const VehicleWheelState& other) const noexcept {
    return suspensionLength == other.suspensionLength &&
           suspensionSpeed == other.suspensionSpeed &&
           pendingCorrection == other.pendingCorrection &&
           contactNormal.x == other.contactNormal.x &&
           contactNormal.y == other.contactNormal.y &&
           contactNormal.z == other.contactNormal.z &&
           contactPoint.x == other.contactPoint.x &&
           contactPoint.y == other.contactPoint.y &&
           contactPoint.z == other.contactPoint.z && contact == other.contact &&
           slipping == other.slipping;
  }
};

struct VehicleState final {
  gbx::Float3 position;
  gbx::Float3 groundNormal{0.0F, 1.0F, 0.0F};
  std::array<float, 4> orientationWxyz{1.0F, 0.0F, 0.0F, 0.0F};
  gbx::Float3 angularVelocity;
  gbx::Float3 cachedLocalAngularVelocity;
  gbx::Float3 priorLocalForcePerInertialMass;
  std::array<VehicleWheelState, 4> wheels;
  float smoothedSteering = 0.0F;
  bool reverseDirectionLatched = false;
  float horizontalVelocityX = 0.0F;
  float horizontalVelocityZ = 0.0F;
  float heading = 0.0F;
  float speed = 0.0F;
  std::int32_t gear = 1;
  float engineRpm = 0.0F;
  float engineTargetRpm = 0.0F;
  float gearShiftSeconds = 0.0F;
  float verticalVelocity = 0.0F;
  float elapsedSeconds = 0.0F;
  float lastWheelContactSeconds = 0.0F;
  float raceTimeSeconds = 0.0F;
  std::uint32_t checkpointCount = 0;
  std::uint32_t completedLaps = 0;
  bool grounded = false;
  bool bodyCollisionPulse = false;
  bool finished = false;
};

struct VehicleInitializeResult final {
  std::string error;

  [[nodiscard]] explicit operator bool() const noexcept { return error.empty(); }
};

class VehicleSimulation final {
public:
  [[nodiscard]] VehicleInitializeResult initialize(const content::TrackScene& scene,
                                                   const content::TrackCollision& collision,
                                                   const gbx::VehicleTuning& tuning,
                                                   const gbx::SolidModel* vehicleSolid = nullptr);
  void reset() noexcept;
  [[nodiscard]] bool restore(VehicleState state) noexcept;
  void update(float deltaSeconds, VehicleControl control) noexcept;

  [[nodiscard]] const VehicleState& state() const noexcept { return state_; }
  [[nodiscard]] float frontWheelSteeringRadians() const noexcept;
  [[nodiscard]] bool initialized() const noexcept { return collision_ != nullptr; }
  [[nodiscard]] std::size_t checkpointTotal() const noexcept { return checkpoints_.size(); }
  [[nodiscard]] std::uint32_t lapTotal() const noexcept { return lapTotal_; }

private:
  struct WaypointTrigger final {
    std::size_t collisionTriggerIndex = 0;
    std::optional<VehicleState> respawnState;
  };

  struct BodyEllipsoid final {
    gbx::Float3 center;
    gbx::Float3 halfAxes;
    std::array<float, 9> rotation{};
  };

  void step(float deltaSeconds, const VehicleControl& control) noexcept;
  void updateDrivetrain(float deltaSeconds, const VehicleControl& control) noexcept;
  void updateRace(gbx::Float3 previousPosition) noexcept;

  const content::TrackCollision* collision_ = nullptr;
  float groundClearance_ = 0.55F;
  std::vector<WaypointTrigger> checkpoints_;
  std::optional<WaypointTrigger> finish_;
  std::uint32_t lapTotal_ = 1;
  bool lapRace_ = false;
  gbx::VehicleTuning tuning_;
  gbx::Float3 centerOfMass_;
  gbx::Float3 inverseInertia_;
  std::vector<BodyEllipsoid> bodyEllipsoids_;
  VehicleState spawnState_;
  VehicleState respawnState_;
  VehicleState state_;
  double stepAccumulatorSeconds_ = 0.0;
};

} // namespace tmnf::gameplay
