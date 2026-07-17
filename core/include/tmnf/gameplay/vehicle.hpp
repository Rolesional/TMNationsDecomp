#pragma once

#include "tmnf/content/track_collision.hpp"
#include "tmnf/content/track_scene.hpp"
#include "tmnf/gbx/solid.hpp"

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

struct VehicleState final {
  gbx::Float3 position;
  gbx::Float3 groundNormal{0.0F, 1.0F, 0.0F};
  float heading = 0.0F;
  float speed = 0.0F;
  float verticalVelocity = 0.0F;
  float elapsedSeconds = 0.0F;
  float raceTimeSeconds = 0.0F;
  std::uint32_t checkpointCount = 0;
  std::uint32_t completedLaps = 0;
  bool grounded = false;
  bool finished = false;
};

struct VehicleInitializeResult final {
  std::string error;

  [[nodiscard]] explicit operator bool() const noexcept { return error.empty(); }
};

class VehicleSimulation final {
public:
  [[nodiscard]] VehicleInitializeResult initialize(const content::TrackScene& scene,
                                                   const content::TrackCollision& collision);
  void reset() noexcept;
  [[nodiscard]] bool restore(VehicleState state) noexcept;
  void update(float deltaSeconds, VehicleControl control) noexcept;

  [[nodiscard]] const VehicleState& state() const noexcept { return state_; }
  [[nodiscard]] bool initialized() const noexcept { return collision_ != nullptr; }
  [[nodiscard]] std::size_t checkpointTotal() const noexcept { return checkpoints_.size(); }
  [[nodiscard]] std::uint32_t lapTotal() const noexcept { return lapTotal_; }

private:
  struct WaypointTrigger final {
    std::size_t collisionTriggerIndex = 0;
    std::optional<VehicleState> respawnState;
  };

  void step(float deltaSeconds, const VehicleControl& control) noexcept;
  void updateRace(gbx::Float3 previousPosition) noexcept;

  const content::TrackCollision* collision_ = nullptr;
  float groundClearance_ = 0.55F;
  std::vector<WaypointTrigger> checkpoints_;
  std::optional<WaypointTrigger> finish_;
  std::uint32_t lapTotal_ = 1;
  bool lapRace_ = false;
  VehicleState spawnState_;
  VehicleState respawnState_;
  VehicleState state_;
};

} // namespace tmnf::gameplay
