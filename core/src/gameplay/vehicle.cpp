#include "tmnf/gameplay/vehicle.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string_view>

namespace tmnf::gameplay {
namespace {

constexpr float kPi = 3.14159265358979323846F;
constexpr float kDefaultVehicleGroundClearance = 0.55F;
constexpr std::uint32_t kChallengeBlockGroundFlag = 1U << 12U;

float clampControl(const float value) noexcept { return std::clamp(value, 0.0F, 1.0F); }

float approachZero(const float value, const float amount) noexcept {
  if (value > amount) {
    return value - amount;
  }
  if (value < -amount) {
    return value + amount;
  }
  return 0.0F;
}

const std::array<float, 12>* spawnTransform(const gbx::BlockInfoMetadata& metadata,
                                            const gbx::ChallengeBlock& block) noexcept {
  const bool ground = (block.flags & kChallengeBlockGroundFlag) != 0U;
  const auto& preferred = ground ? metadata.spawnGround : metadata.spawnAir;
  const auto& fallback = ground ? metadata.spawnAir : metadata.spawnGround;
  const auto& transform = preferred.has_value() ? preferred : fallback;
  return transform.has_value() ? &*transform : nullptr;
}

bool sameCoordinate(const gbx::ChallengeBlock& block,
                    const std::array<std::int32_t, 3>& coordinate) noexcept {
  return coordinate[0] >= 0 && coordinate[1] >= 0 && coordinate[2] >= 0 &&
         static_cast<std::int32_t>(block.coordinate[0]) == coordinate[0] &&
         static_cast<std::int32_t>(block.coordinate[1]) == coordinate[1] &&
         static_cast<std::int32_t>(block.coordinate[2]) == coordinate[2];
}

} // namespace

VehicleInitializeResult VehicleSimulation::initialize(const content::TrackScene& scene,
                                                      const content::TrackCollision& collision) {
  checkpoints_.clear();
  finish_.reset();
  lapRace_ = scene.challenge.isLapRace;
  if (lapRace_ && (scene.challenge.nbLaps <= 0 || scene.challenge.nbLaps > 10000)) {
    return {"challenge lap count is invalid"};
  }
  lapTotal_ = lapRace_ ? static_cast<std::uint32_t>(scene.challenge.nbLaps) : 1U;
  const gbx::ChallengeBlock* start = nullptr;
  const gbx::BlockInfoMetadata* startMetadata = nullptr;
  for (const content::TrackBlockDefinition& definition : scene.blockDefinitions) {
    if (definition.metadata.waypointType != gbx::BlockWaypointType::Start &&
        definition.metadata.waypointType != gbx::BlockWaypointType::StartFinish) {
      continue;
    }
    const auto block = std::ranges::find_if(scene.challenge.blocks, [&](const auto& candidate) {
      return candidate.name == definition.name;
    });
    if (block != scene.challenge.blocks.end()) {
      start = &*block;
      startMetadata = &definition.metadata;
      break;
    }
  }
  for (const gbx::ChallengeBlock& block : scene.challenge.blocks) {
    if (start != nullptr) {
      break;
    }
    if (block.name.find("StartLine") != std::string::npos) {
      start = &block;
      break;
    }
  }
  if (start == nullptr) {
    for (const gbx::ChallengeBlock& block : scene.challenge.blocks) {
      if (block.name.ends_with("Start")) {
        start = &block;
        break;
      }
    }
  }
  if (start == nullptr) {
    return {"challenge has no start block"};
  }
  gbx::Float3 localPosition{16.0F, 0.0F, 24.0F};
  gbx::Float3 localForward{0.0F, 0.0F, 1.0F};
  bool hasRecoveredSpawn = false;
  if (startMetadata != nullptr) {
    if (const auto* transform = spawnTransform(*startMetadata, *start); transform != nullptr) {
      localPosition = {(*transform)[9], (*transform)[10], (*transform)[11]};
      localForward = {(*transform)[2], (*transform)[5], (*transform)[8]};
      hasRecoveredSpawn = true;
    }
  }
  const gbx::Float3 center = content::placeTrackPoint(localPosition, *start);
  const std::optional<content::TrackGroundHit> ground =
      content::sampleTrackGround(collision, center.x, center.z, center.y - 16.0F, center.y + 32.0F);
  if (!ground.has_value()) {
    return {"start block has no collision surface"};
  }
  groundClearance_ = kDefaultVehicleGroundClearance;
  if (hasRecoveredSpawn) {
    const float recoveredClearance = center.y - ground->height;
    if (std::isfinite(recoveredClearance) && recoveredClearance >= 0.05F &&
        recoveredClearance <= 3.0F) {
      groundClearance_ = recoveredClearance;
    }
  }
  const gbx::Float3 forward = content::rotateTrackVector(localForward, start->direction);

  const auto definitionFor = [&](const gbx::ChallengeBlock& block,
                                 const gbx::BlockWaypointType type) {
    return std::ranges::find_if(scene.blockDefinitions, [&](const auto& definition) {
      return definition.name == block.name && definition.metadata.waypointType == type;
    });
  };
  const auto triggerForBlock = [&](const std::size_t blockIndex) {
    return std::ranges::find_if(
        collision.triggers, [&](const auto& trigger) { return trigger.blockIndex == blockIndex; });
  };
  const auto waypointRespawnState =
      [&](const gbx::ChallengeBlock& block,
          const gbx::BlockInfoMetadata& metadata) -> std::optional<VehicleState> {
    const auto* transform = spawnTransform(metadata, block);
    if (transform == nullptr) {
      return std::nullopt;
    }
    const gbx::Float3 position =
        content::placeTrackPoint({(*transform)[9], (*transform)[10], (*transform)[11]}, block);
    const gbx::Float3 direction = content::rotateTrackVector(
        {(*transform)[2], (*transform)[5], (*transform)[8]}, block.direction);
    VehicleState respawn;
    respawn.position = position;
    respawn.heading = std::atan2(direction.x, direction.z);
    const std::optional<content::TrackGroundHit> respawnGround = content::sampleTrackGround(
        collision, position.x, position.z, position.y - 16.0F, position.y + 32.0F);
    if (respawnGround.has_value()) {
      respawn.position.y = respawnGround->height + groundClearance_;
      respawn.groundNormal = respawnGround->normal;
      respawn.grounded = true;
    }
    return respawn;
  };
  for (const auto& coordinate : scene.challenge.checkpoints) {
    const auto checkpoint = std::ranges::find_if(scene.challenge.blocks, [&](const auto& block) {
      if (!sameCoordinate(block, coordinate)) {
        return false;
      }
      return definitionFor(block, gbx::BlockWaypointType::Checkpoint) !=
             scene.blockDefinitions.end();
    });
    if (checkpoint == scene.challenge.blocks.end()) {
      return {"ordered checkpoint has no matching waypoint block"};
    }
    const std::size_t blockIndex =
        static_cast<std::size_t>(checkpoint - scene.challenge.blocks.begin());
    const auto trigger = triggerForBlock(blockIndex);
    if (trigger == collision.triggers.end()) {
      return {"checkpoint block has no trigger mesh"};
    }
    const auto definition = definitionFor(*checkpoint, gbx::BlockWaypointType::Checkpoint);
    std::optional<VehicleState> respawn;
    if (!definition->metadata.noRespawn) {
      respawn = waypointRespawnState(*checkpoint, definition->metadata);
      if (!respawn.has_value()) {
        return {"respawnable checkpoint has no spawn transform"};
      }
    }
    checkpoints_.push_back(
        {static_cast<std::size_t>(trigger - collision.triggers.begin()), std::move(respawn)});
  }
  for (std::size_t blockIndex = 0; blockIndex < scene.challenge.blocks.size(); ++blockIndex) {
    const gbx::ChallengeBlock& block = scene.challenge.blocks[blockIndex];
    auto definition = definitionFor(block, gbx::BlockWaypointType::Finish);
    if (definition == scene.blockDefinitions.end()) {
      definition = definitionFor(block, gbx::BlockWaypointType::StartFinish);
    }
    if (definition == scene.blockDefinitions.end()) {
      continue;
    }
    const auto trigger = triggerForBlock(blockIndex);
    if (trigger == collision.triggers.end()) {
      return {"finish block has no trigger mesh"};
    }
    finish_ = WaypointTrigger{static_cast<std::size_t>(trigger - collision.triggers.begin()),
                              waypointRespawnState(block, definition->metadata)};
    break;
  }
  if (!finish_.has_value()) {
    return {"challenge has no finish trigger"};
  }

  spawnState_ = {};
  spawnState_.position = {center.x, ground->height + groundClearance_, center.z};
  spawnState_.groundNormal = ground->normal;
  spawnState_.heading = std::atan2(forward.x, forward.z);
  spawnState_.grounded = true;
  respawnState_ = spawnState_;
  state_ = {};
  collision_ = &collision;
  reset();
  return {};
}

void VehicleSimulation::reset() noexcept {
  VehicleState respawn = respawnState_;
  respawn.elapsedSeconds = state_.elapsedSeconds;
  respawn.raceTimeSeconds = state_.raceTimeSeconds;
  respawn.checkpointCount = state_.checkpointCount;
  respawn.completedLaps = state_.completedLaps;
  respawn.finished = state_.finished;
  state_ = respawn;
}

bool VehicleSimulation::restore(VehicleState state) noexcept {
  if (collision_ == nullptr || !std::isfinite(state.position.x) ||
      !std::isfinite(state.position.y) || !std::isfinite(state.position.z) ||
      !std::isfinite(state.heading) || !std::isfinite(state.speed) ||
      !std::isfinite(state.verticalVelocity) || !std::isfinite(state.elapsedSeconds) ||
      !std::isfinite(state.raceTimeSeconds) || state.raceTimeSeconds < 0.0F ||
      state.checkpointCount > checkpoints_.size() || state.completedLaps > lapTotal_ ||
      (!lapRace_ && state.completedLaps != 0U) ||
      (lapRace_ && !state.finished && state.completedLaps >= lapTotal_) ||
      (state.finished && (state.checkpointCount != checkpoints_.size() ||
                          (lapRace_ && state.completedLaps != lapTotal_))) ||
      std::abs(state.speed) > 200.0F || std::abs(state.verticalVelocity) > 500.0F) {
    return false;
  }
  state_ = state;
  respawnState_ = spawnState_;
  if (lapRace_ && state.completedLaps > 0U && state.checkpointCount == 0U &&
      finish_->respawnState.has_value()) {
    respawnState_ = *finish_->respawnState;
  }
  for (std::size_t index = 0; index < state.checkpointCount; ++index) {
    if (checkpoints_[index].respawnState.has_value()) {
      respawnState_ = *checkpoints_[index].respawnState;
    }
  }
  return true;
}

void VehicleSimulation::update(const float deltaSeconds, VehicleControl control) noexcept {
  if (collision_ == nullptr || !std::isfinite(deltaSeconds) || deltaSeconds <= 0.0F) {
    return;
  }
  control.steering = std::clamp(control.steering, -1.0F, 1.0F);
  control.throttle = clampControl(control.throttle);
  control.brake = clampControl(control.brake);
  float remaining = std::min(deltaSeconds, 0.1F);
  constexpr float kMaximumStep = 1.0F / 120.0F;
  while (remaining > 0.0F) {
    const float stepSeconds = std::min(remaining, kMaximumStep);
    step(stepSeconds, control);
    remaining -= stepSeconds;
  }
}

void VehicleSimulation::step(const float deltaSeconds, const VehicleControl& control) noexcept {
  constexpr float kForwardAcceleration = 25.0F;
  constexpr float kReverseAcceleration = 12.0F;
  constexpr float kBrakeAcceleration = 36.0F;
  constexpr float kRollingResistance = 1.25F;
  constexpr float kAerodynamicDrag = 0.0035F;
  constexpr float kMaximumForwardSpeed = 92.0F;
  constexpr float kMaximumReverseSpeed = 14.0F;
  constexpr float kMaximumSteeringRate = 1.65F;
  constexpr float kGravity = 9.81F;
  constexpr float kBodyCollisionRadius = 0.9F;
  constexpr float kBodyCollisionCenterHeight = 0.45F;
  const gbx::Float3 previousPosition = state_.position;

  if (state_.grounded) {
    if (control.throttle > 0.0F) {
      state_.speed += control.throttle *
                      (state_.speed < -0.1F ? kBrakeAcceleration : kForwardAcceleration) *
                      deltaSeconds;
    }
    if (control.brake > 0.0F) {
      state_.speed -= control.brake *
                      (state_.speed > 0.1F ? kBrakeAcceleration : kReverseAcceleration) *
                      deltaSeconds;
    }
    if (control.throttle <= 0.001F && control.brake <= 0.001F) {
      state_.speed = approachZero(state_.speed, kRollingResistance * deltaSeconds);
    }
    state_.speed -= state_.speed * std::abs(state_.speed) * kAerodynamicDrag * deltaSeconds;
    state_.speed = std::clamp(state_.speed, -kMaximumReverseSpeed, kMaximumForwardSpeed);

    const float steeringAuthority = std::min(std::abs(state_.speed) / 12.0F, 1.0F);
    const float reverseSign = state_.speed < 0.0F ? -1.0F : 1.0F;
    state_.heading +=
        control.steering * steeringAuthority * reverseSign * kMaximumSteeringRate * deltaSeconds;
    if (state_.heading > kPi) {
      state_.heading -= 2.0F * kPi;
    } else if (state_.heading < -kPi) {
      state_.heading += 2.0F * kPi;
    }
  }

  const float forwardX = std::sin(state_.heading);
  const float forwardZ = std::cos(state_.heading);
  state_.position.x += forwardX * state_.speed * deltaSeconds;
  state_.position.z += forwardZ * state_.speed * deltaSeconds;

  bool hitWall = false;
  for (int iteration = 0; iteration < 4; ++iteration) {
    const gbx::Float3 bodyCenter{state_.position.x, state_.position.y + kBodyCollisionCenterHeight,
                                 state_.position.z};
    const std::optional<content::TrackWallContact> contact =
        content::sampleTrackWallContact(*collision_, bodyCenter, kBodyCollisionRadius);
    if (!contact.has_value()) {
      break;
    }
    constexpr float kContactSlop = 0.001F;
    state_.position.x += contact->normal.x * (contact->penetration + kContactSlop);
    state_.position.z += contact->normal.z * (contact->penetration + kContactSlop);
    const float normalVelocity =
        (forwardX * contact->normal.x + forwardZ * contact->normal.z) * state_.speed;
    if (normalVelocity < 0.0F) {
      const float retainedSpeed =
          std::max(0.0F, std::abs(state_.speed) - std::abs(normalVelocity) * 0.85F);
      state_.speed = std::copysign(retainedSpeed, state_.speed);
      hitWall = true;
    }
  }
  if (hitWall && std::abs(state_.speed) < 0.05F) {
    state_.speed = 0.0F;
  }

  if (state_.grounded) {
    const std::optional<content::TrackGroundHit> ground =
        content::sampleTrackGround(*collision_, state_.position.x, state_.position.z,
                                   state_.position.y - 3.0F, state_.position.y + 4.0F);
    if (ground.has_value()) {
      state_.position.y = ground->height + groundClearance_;
      state_.groundNormal = ground->normal;
      state_.verticalVelocity = 0.0F;
    } else {
      state_.grounded = false;
    }
  }
  if (!state_.grounded) {
    state_.verticalVelocity -= kGravity * deltaSeconds;
    state_.position.y += state_.verticalVelocity * deltaSeconds;
    const std::optional<content::TrackGroundHit> ground =
        content::sampleTrackGround(*collision_, state_.position.x, state_.position.z,
                                   state_.position.y - 1.5F, state_.position.y + groundClearance_);
    if (ground.has_value() && state_.verticalVelocity <= 0.0F &&
        state_.position.y <= ground->height + groundClearance_) {
      state_.position.y = ground->height + groundClearance_;
      state_.groundNormal = ground->normal;
      state_.verticalVelocity = 0.0F;
      state_.grounded = true;
    }
  }
  state_.elapsedSeconds += deltaSeconds;
  if (!state_.finished) {
    state_.raceTimeSeconds += deltaSeconds;
  }
  updateRace(previousPosition);
  if (state_.position.y < -64.0F || !std::isfinite(state_.position.x) ||
      !std::isfinite(state_.position.y) || !std::isfinite(state_.position.z)) {
    reset();
  }
}

void VehicleSimulation::updateRace(const gbx::Float3 previousPosition) noexcept {
  const auto crossed = [&](const WaypointTrigger& trigger) {
    return trigger.collisionTriggerIndex < collision_->triggers.size() &&
           content::segmentIntersectsTrackTrigger(
               collision_->triggers[trigger.collisionTriggerIndex], previousPosition,
               state_.position);
  };

  if (state_.checkpointCount < checkpoints_.size() &&
      crossed(checkpoints_[state_.checkpointCount])) {
    if (checkpoints_[state_.checkpointCount].respawnState.has_value()) {
      respawnState_ = *checkpoints_[state_.checkpointCount].respawnState;
    }
    ++state_.checkpointCount;
  }
  if (!state_.finished && state_.checkpointCount == checkpoints_.size() && finish_.has_value() &&
      crossed(*finish_)) {
    if (!lapRace_) {
      state_.finished = true;
      return;
    }
    ++state_.completedLaps;
    if (finish_->respawnState.has_value()) {
      respawnState_ = *finish_->respawnState;
    }
    if (state_.completedLaps >= lapTotal_) {
      state_.finished = true;
    } else {
      state_.checkpointCount = 0;
    }
  }
}

} // namespace tmnf::gameplay
