#include "tmnf/gameplay/vehicle.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string_view>

namespace tmnf::gameplay {
namespace {

constexpr float kStadiumGravity = 10.0F;
constexpr float kDegreesToRadians = 0.01745329251994329577F;
constexpr float kDefaultVehicleGroundClearance = 0.55F;
constexpr std::uint32_t kChallengeBlockGroundFlag = 1U << 12U;
constexpr float kGearShiftSeconds = 0.025F;
constexpr float kDirectionChangeRpm = 1'000.0F;
// CPlugVehicleVisModelShared 0x0A039005: FLSurf, FRSurf, RRSurf, RLSurf.
constexpr std::array<gbx::Float3, 4> kWheelCenters{{
    {0.863012016F, 0.352499992F, 1.78208899F},
    {-0.862990022F, 0.352499992F, 1.78208899F},
    {-0.884999990F, 0.352503985F, -1.20550203F},
    {0.885002017F, 0.352503985F, -1.20550203F},
}};
constexpr gbx::Float3 kWheelHalfAxes{0.182F, 0.364F, 0.364F};

using RotationMatrix = std::array<float, 9>;

gbx::Float3 add(const gbx::Float3 left, const gbx::Float3 right) noexcept {
  return {left.x + right.x, left.y + right.y, left.z + right.z};
}

gbx::Float3 subtract(const gbx::Float3 left, const gbx::Float3 right) noexcept {
  return {left.x - right.x, left.y - right.y, left.z - right.z};
}

gbx::Float3 multiply(const gbx::Float3 value, const float scale) noexcept {
  return {value.x * scale, value.y * scale, value.z * scale};
}

float dot(const gbx::Float3 left, const gbx::Float3 right) noexcept {
  return left.x * right.x + left.y * right.y + left.z * right.z;
}

gbx::Float3 cross(const gbx::Float3 left, const gbx::Float3 right) noexcept {
  return {left.y * right.z - left.z * right.y, left.z * right.x - left.x * right.z,
          left.x * right.y - left.y * right.x};
}

gbx::Float3 normalize(const gbx::Float3 value) noexcept {
  const float lengthSquared = dot(value, value);
  return lengthSquared > 1.0e-10F ? multiply(value, 1.0F / std::sqrt(lengthSquared))
                                  : gbx::Float3{0.0F, 1.0F, 0.0F};
}

gbx::Float3 reduceContactCorrections(
    const std::vector<content::TrackEllipsoidContact>& contacts) noexcept {
  if (contacts.empty()) {
    return {};
  }
  gbx::Float3 correction = contacts.front().correction;
  constexpr float kDirectionEpsilon = 9.99999944e-11F; // 0068FF90: 0x2EDBE6FE.
  for (std::size_t index = 1; index < contacts.size(); ++index) {
    const gbx::Float3 next = contacts[index].correction;
    const float alignment = dot(correction, next);
    const float lengthSquared = dot(correction, correction);
    if (alignment > 0.0F && lengthSquared > kDirectionEpsilon) {
      correction = subtract(
          correction,
          multiply(correction, std::min(alignment, lengthSquared) / lengthSquared));
    }
    correction = add(correction, next);
  }
  constexpr float kContactSlop = 0.01F;
  const float lengthSquared = dot(correction, correction);
  if (lengthSquared <= kContactSlop * kContactSlop) {
    return {};
  }
  const float length = std::sqrt(lengthSquared);
  return multiply(correction, 1.0F - kContactSlop / length);
}

std::array<float, 4> normalizeQuaternion(std::array<float, 4> value) noexcept {
  const float lengthSquared =
      value[0] * value[0] + value[1] * value[1] + value[2] * value[2] + value[3] * value[3];
  if (lengthSquared <= 1.0e-10F) {
    return {1.0F, 0.0F, 0.0F, 0.0F};
  }
  const float inverseLength = 1.0F / std::sqrt(lengthSquared);
  for (float& component : value) {
    component *= inverseLength;
  }
  return value;
}

std::array<float, 4> quaternionMultiply(const std::array<float, 4>& left,
                                        const std::array<float, 4>& right) noexcept {
  return {left[0] * right[0] - left[1] * right[1] - left[2] * right[2] -
              left[3] * right[3],
          left[0] * right[1] + left[1] * right[0] + left[2] * right[3] -
              left[3] * right[2],
          left[0] * right[2] - left[1] * right[3] + left[2] * right[0] +
              left[3] * right[1],
          left[0] * right[3] + left[1] * right[2] - left[2] * right[1] +
              left[3] * right[0]};
}

RotationMatrix rotationMatrix(const std::array<float, 4>& quaternion) noexcept {
  const float w = quaternion[0];
  const float x = quaternion[1];
  const float y = quaternion[2];
  const float z = quaternion[3];
  return {1.0F - 2.0F * (y * y + z * z), 2.0F * (x * y - z * w),
          2.0F * (x * z + y * w),       2.0F * (x * y + z * w),
          1.0F - 2.0F * (x * x + z * z), 2.0F * (y * z - x * w),
          2.0F * (x * z - y * w),       2.0F * (y * z + x * w),
          1.0F - 2.0F * (x * x + y * y)};
}

gbx::Float3 rotate(const RotationMatrix& rotation, const gbx::Float3 value) noexcept {
  return {rotation[0] * value.x + rotation[1] * value.y + rotation[2] * value.z,
          rotation[3] * value.x + rotation[4] * value.y + rotation[5] * value.z,
          rotation[6] * value.x + rotation[7] * value.y + rotation[8] * value.z};
}

gbx::Float3 inverseRotate(const RotationMatrix& rotation, const gbx::Float3 value) noexcept {
  return {rotation[0] * value.x + rotation[3] * value.y + rotation[6] * value.z,
          rotation[1] * value.x + rotation[4] * value.y + rotation[7] * value.z,
          rotation[2] * value.x + rotation[5] * value.y + rotation[8] * value.z};
}

RotationMatrix multiplyRotation(const RotationMatrix& left,
                                const RotationMatrix& right) noexcept {
  RotationMatrix result{};
  for (std::size_t row = 0; row < 3U; ++row) {
    for (std::size_t column = 0; column < 3U; ++column) {
      result[row * 3U + column] =
          left[row * 3U] * right[column] +
          left[row * 3U + 1U] * right[3U + column] +
          left[row * 3U + 2U] * right[6U + column];
    }
  }
  return result;
}

std::array<float, 4> quaternionFromMatrix(const RotationMatrix& rotation) noexcept {
  std::array<float, 4> result{};
  const float trace = rotation[0] + rotation[4] + rotation[8];
  if (trace > 0.0F) {
    const float scale = 2.0F * std::sqrt(trace + 1.0F);
    result = {0.25F * scale, (rotation[7] - rotation[5]) / scale,
              (rotation[2] - rotation[6]) / scale, (rotation[3] - rotation[1]) / scale};
  } else if (rotation[0] > rotation[4] && rotation[0] > rotation[8]) {
    const float scale = 2.0F * std::sqrt(1.0F + rotation[0] - rotation[4] - rotation[8]);
    result = {(rotation[7] - rotation[5]) / scale, 0.25F * scale,
              (rotation[1] + rotation[3]) / scale, (rotation[2] + rotation[6]) / scale};
  } else if (rotation[4] > rotation[8]) {
    const float scale = 2.0F * std::sqrt(1.0F + rotation[4] - rotation[0] - rotation[8]);
    result = {(rotation[2] - rotation[6]) / scale, (rotation[1] + rotation[3]) / scale,
              0.25F * scale, (rotation[5] + rotation[7]) / scale};
  } else {
    const float scale = 2.0F * std::sqrt(1.0F + rotation[8] - rotation[0] - rotation[4]);
    result = {(rotation[3] - rotation[1]) / scale, (rotation[2] + rotation[6]) / scale,
              (rotation[5] + rotation[7]) / scale, 0.25F * scale};
  }
  return normalizeQuaternion(result);
}

std::array<float, 4> orientationFromPose(const float heading,
                                         const gbx::Float3 groundNormal) noexcept {
  const gbx::Float3 up = normalize(groundNormal);
  const gbx::Float3 horizontalForward{std::sin(heading), 0.0F, std::cos(heading)};
  gbx::Float3 forward = subtract(horizontalForward, multiply(up, dot(horizontalForward, up)));
  if (dot(forward, forward) <= 1.0e-10F) {
    forward = horizontalForward;
  }
  forward = normalize(forward);
  const gbx::Float3 right = normalize(cross(up, forward));
  forward = normalize(cross(right, up));
  return quaternionFromMatrix({right.x, up.x, forward.x, right.y, up.y, forward.y,
                               right.z, up.z, forward.z});
}

void syncPoseFromOrientation(VehicleState& state) noexcept {
  const RotationMatrix rotation = rotationMatrix(state.orientationWxyz);
  state.groundNormal = {rotation[1], rotation[4], rotation[7]};
  const float horizontalForwardSquared = rotation[2] * rotation[2] + rotation[8] * rotation[8];
  if (horizontalForwardSquared > 1.0e-10F) {
    state.heading = std::atan2(rotation[2], rotation[8]);
  }
}

void resetVehicleDynamics(VehicleState& state, const gbx::VehicleTuning& tuning) noexcept {
  state.orientationWxyz = orientationFromPose(state.heading, state.groundNormal);
  state.angularVelocity = {};
  state.cachedLocalAngularVelocity = {};
  state.priorLocalForcePerInertialMass = {};
  state.smoothedSteering = 0.0F;
  state.reverseDirectionLatched = false;
  state.lastWheelContactSeconds = 0.0F;
  state.bodyCollisionPulse = false;
  const RotationMatrix rotation = rotationMatrix(state.orientationWxyz);
  for (std::size_t index = 0; index < state.wheels.size(); ++index) {
    VehicleWheelState& wheel = state.wheels[index];
    wheel = {};
    wheel.suspensionLength = tuning.suspensionRest;
    wheel.contactPoint = add(
        state.position,
        rotate(rotation,
               subtract(kWheelCenters[index],
                        {0.0F, tuning.suspensionRest + kWheelHalfAxes.y, 0.0F})));
    // 009D6430 rebuilds each wheel at rest and clears support/contact on reset.
    wheel.contact = false;
  }
}

gbx::Float3 vehicleCenterOfMass(const gbx::VehicleTuning& tuning) noexcept {
  float minimumX = kWheelCenters.front().x;
  float maximumX = minimumX;
  float minimumZ = kWheelCenters.front().z;
  float maximumZ = minimumZ;
  float bottomSum = 0.0F;
  for (const gbx::Float3 center : kWheelCenters) {
    minimumX = std::min(minimumX, center.x);
    maximumX = std::max(maximumX, center.x);
    minimumZ = std::min(minimumZ, center.z);
    maximumZ = std::max(maximumZ, center.z);
    bottomSum += center.y - kWheelHalfAxes.y;
  }
  return {(minimumX + maximumX) * 0.5F,
          bottomSum / static_cast<float>(kWheelCenters.size()) + tuning.centerOfMassDownUp,
          (minimumZ + maximumZ) * 0.5F +
              tuning.centerOfMassAftForce * (maximumZ - minimumZ) * 0.5F};
}

gbx::Float3 vehicleInverseInertia(const gbx::VehicleTuning& tuning) noexcept {
  const float width = 2.0F * tuning.inertiaHalfDiagonal[0];
  const float height = 2.0F * tuning.inertiaHalfDiagonal[1];
  const float depth = 2.0F * tuning.inertiaHalfDiagonal[2];
  const float scale = 12.0F / tuning.inertiaMass;
  return {scale / (height * height + depth * depth),
          scale / (width * width + depth * depth),
          scale / (width * width + height * height)};
}

void applyRigidBodyImpulse(gbx::Float3& linearVelocity, gbx::Float3& angularVelocity,
                           const gbx::Float3 impulse, const gbx::Float3 lever,
                           const RotationMatrix& rotation, const gbx::Float3 inverseInertia,
                           const gbx::VehicleTuning& tuning) noexcept {
  const float oldLinearSpeedSquared = dot(linearVelocity, linearVelocity);
  gbx::Float3 nextLinearVelocity =
      add(linearVelocity, multiply(impulse, 1.0F / tuning.mass));
  const float nextLinearSpeedSquared = dot(nextLinearVelocity, nextLinearVelocity);
  if (nextLinearSpeedSquared > oldLinearSpeedSquared &&
      nextLinearSpeedSquared - oldLinearSpeedSquared >
          tuning.linearSpeed2PositiveDeltaMax) {
    nextLinearVelocity = {};
  }
  linearVelocity = nextLinearVelocity;

  gbx::Float3 localAngularVelocity = inverseRotate(rotation, angularVelocity);
  const gbx::Float3 localAngularImpulse =
      inverseRotate(rotation, cross(lever, impulse));
  localAngularVelocity =
      add(localAngularVelocity,
          {localAngularImpulse.x * inverseInertia.x * tuning.angularSpeedYImpulseScale,
           localAngularImpulse.y * inverseInertia.y * tuning.angularSpeedYImpulseScale,
           localAngularImpulse.z * inverseInertia.z * tuning.angularSpeedYImpulseScale});
  const float angularSpeedSquared = dot(localAngularVelocity, localAngularVelocity);
  const float angularClampSquared = tuning.angularSpeedClamp * tuning.angularSpeedClamp;
  if (angularSpeedSquared > angularClampSquared) {
    localAngularVelocity =
        multiply(localAngularVelocity, tuning.angularSpeedClamp / std::sqrt(angularSpeedSquared));
  }
  angularVelocity = rotate(rotation, localAngularVelocity);
}

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

float adherenceFromRatio(const float ratio, const float minimum,
                         const float maximum) noexcept {
  if (ratio < minimum) {
    return 0.0F;
  }
  if (ratio > maximum) {
    return 1.0F;
  }
  constexpr float kHalfPi = 1.57079632679489661923F;
  return 1.0F - std::cos((ratio - minimum) / (maximum - minimum) * kHalfPi);
}

bool validCurve(const gbx::RealCurve& curve) noexcept {
  return !curve.x.empty() && curve.x.size() == curve.y.size() &&
         std::ranges::is_sorted(curve.x) &&
         std::ranges::all_of(curve.x, [](const float value) { return std::isfinite(value); }) &&
         std::ranges::all_of(curve.y, [](const float value) { return std::isfinite(value); });
}

bool validDrivetrain(const gbx::VehicleTuning& tuning) noexcept {
  const std::size_t gearCount = tuning.m6GearRatio.size();
  if (!std::isfinite(tuning.m6MaxRpm) || tuning.m6MaxRpm <= 0.0F || gearCount < 2U ||
      gearCount > static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()) ||
      tuning.m6MaxRpmByGear.size() != gearCount ||
      tuning.m6MinRpmByGear.size() != gearCount ||
      tuning.m6RpmWantedOnGearUp.size() != gearCount ||
      !std::isfinite(tuning.m6BurnoutRpmAcceleration) ||
      tuning.m6BurnoutRpmAcceleration <= 0.0F ||
      !std::isfinite(tuning.m6AirRpmAcceleration) || tuning.m6AirRpmAcceleration <= 0.0F ||
      !std::isfinite(tuning.m6AirRpmDeadening) || tuning.m6AirRpmDeadening <= 0.0F ||
      !std::isfinite(tuning.m6RpmLossOnGearUp) || tuning.m6RpmLossOnGearUp <= 0.0F ||
      !std::isfinite(tuning.m6RpmGainOnGearDown) || tuning.m6RpmGainOnGearDown <= 0.0F ||
      !std::isfinite(tuning.m6RpmGainOnTakeOff) || tuning.m6RpmGainOnTakeOff <= 0.0F ||
      !std::isfinite(tuning.m6RpmLossOnTakeOffFinished) ||
      tuning.m6RpmLossOnTakeOffFinished <= 0.0F) {
    return false;
  }
  for (std::size_t gear = 0; gear < gearCount; ++gear) {
    const float ratio = tuning.m6GearRatio[gear];
    const float maximum = tuning.m6MaxRpmByGear[gear];
    const float minimum = tuning.m6MinRpmByGear[gear];
    const float wanted = tuning.m6RpmWantedOnGearUp[gear];
    if (!std::isfinite(ratio) || ratio <= 0.0F || !std::isfinite(maximum) ||
        !std::isfinite(minimum) || !std::isfinite(wanted) || minimum < 0.0F ||
        maximum > 1.0F || wanted < 0.0F || wanted > 1.0F || minimum > maximum) {
      return false;
    }
  }
  for (std::size_t gear = 1U; gear + 1U < gearCount; ++gear) {
    if (tuning.m6MaxRpmByGear[gear] * tuning.m6GearRatio[gear + 1U] <=
        tuning.m6MinRpmByGear[gear + 1U] * tuning.m6GearRatio[gear]) {
      return false;
    }
  }
  return true;
}

float sampleCurve(const gbx::RealCurve& curve, const float coordinate) noexcept {
  if (coordinate <= curve.x.front()) {
    return curve.y.front();
  }
  if (coordinate >= curve.x.back()) {
    return curve.y.back();
  }
  const auto right = std::upper_bound(curve.x.begin(), curve.x.end(), coordinate);
  const std::size_t rightIndex = static_cast<std::size_t>(right - curve.x.begin());
  const std::size_t leftIndex = rightIndex - 1U;
  const float span = curve.x[rightIndex] - curve.x[leftIndex];
  if (span <= 0.0F) {
    return curve.y[rightIndex];
  }
  const float fraction = (coordinate - curve.x[leftIndex]) / span;
  // ponytail: the active Stadium force curves need only linear interpolation;
  // add other CFuncKeysReal modes when a recovered runtime tuning uses them.
  return std::lerp(curve.y[leftIndex], curve.y[rightIndex], fraction);
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
                                                      const content::TrackCollision& collision,
                                                      const gbx::VehicleTuning& tuning,
                                                      const gbx::SolidModel* const vehicleSolid) {
  if (!std::isfinite(tuning.mass) || tuning.mass <= 0.0F ||
      !std::isfinite(tuning.maxSpeed) || tuning.maxSpeed <= 0.0F ||
      !std::isfinite(tuning.reverseMaxSpeed) || tuning.reverseMaxSpeed <= 0.0F ||
      !std::isfinite(tuning.suspensionSpring) || tuning.suspensionSpring <= 0.0F ||
      !std::isfinite(tuning.suspensionDamping) || tuning.suspensionDamping < 0.0F ||
      !std::isfinite(tuning.suspensionMinimum) ||
      !std::isfinite(tuning.suspensionMaximum) ||
      !std::isfinite(tuning.suspensionRest) ||
      tuning.suspensionMinimum > tuning.suspensionRest ||
      tuning.suspensionRest > tuning.suspensionMaximum ||
      !std::isfinite(tuning.absorbTension) || tuning.absorbTension < 0.0F ||
      !std::isfinite(tuning.centerOfMassAftForce) ||
      !std::isfinite(tuning.centerOfMassDownUp) ||
      !std::isfinite(tuning.inertiaMass) || tuning.inertiaMass <= 0.0F ||
      !std::ranges::all_of(tuning.inertiaHalfDiagonal, [](const float value) {
        return std::isfinite(value) && value > 0.0F;
      }) ||
      tuning.shockModel != 2 ||
      !std::isfinite(tuning.gravityCoef) || tuning.gravityCoef <= 0.0F ||
      !std::isfinite(tuning.gravityCoefAir) || tuning.gravityCoefAir <= 0.0F ||
      !std::isfinite(tuning.steerRadiusMin) || tuning.steerRadiusMin <= 0.0F ||
      !std::isfinite(tuning.steerRadiusCoef) || tuning.steerRadiusCoef < 0.0F ||
      !std::isfinite(tuning.steerSpeed) || tuning.steerSpeed <= 0.0F ||
      tuning.steerModel != 5 || !std::isfinite(tuning.steerGroundTorque) ||
      tuning.steerGroundTorque < 0.0F ||
      !validCurve(tuning.visualSteerAngleFromSpeed) ||
      !std::isfinite(tuning.linearFluidFrictionCoef) ||
      tuning.linearFluidFrictionCoef < 0.0F ||
      !std::isfinite(tuning.angularFluidFrictionCoef1) ||
      tuning.angularFluidFrictionCoef1 < 0.0F ||
      !std::isfinite(tuning.angularFluidFrictionCoef2) ||
      tuning.angularFluidFrictionCoef2 < 0.0F ||
      tuning.airControlDuration < 0 ||
      !std::isfinite(tuning.maxAngularSpeedYAirControl) ||
      tuning.maxAngularSpeedYAirControl <= 0.0F ||
      !validCurve(tuning.airControlZCoefFromAngularSpeed) ||
      !std::isfinite(tuning.wheelRestCoefConcrete) ||
      !std::isfinite(tuning.wheelRestCoefMetal) ||
      !std::isfinite(tuning.angularSpeedYImpulseScale) ||
      tuning.angularSpeedYImpulseScale < 0.0F ||
      !std::isfinite(tuning.angularSpeedClamp) || tuning.angularSpeedClamp <= 0.0F ||
      !std::isfinite(tuning.linearSpeed2PositiveDeltaMax) ||
      tuning.linearSpeed2PositiveDeltaMax <= 0.0F || !std::isfinite(tuning.steerLowSpeed) ||
      tuning.steerLowSpeed <= 0.0F ||
      !std::isfinite(tuning.steerAngleMax) || tuning.steerAngleMax < 0.0F ||
      !std::isfinite(tuning.lateralSlopeAdherenceMin) ||
      !std::isfinite(tuning.lateralSlopeAdherenceMax) ||
      tuning.lateralSlopeAdherenceMin < 0.0F ||
      tuning.lateralSlopeAdherenceMin >= tuning.lateralSlopeAdherenceMax ||
      !std::isfinite(tuning.axialSlopeAdherenceMin) ||
      !std::isfinite(tuning.axialSlopeAdherenceMax) ||
      tuning.axialSlopeAdherenceMin < 0.0F ||
      tuning.axialSlopeAdherenceMin >= tuning.axialSlopeAdherenceMax ||
      tuning.axialSlopeAdherenceMax > 1.0F ||
      !std::isfinite(tuning.sideFriction1) || tuning.sideFriction1 <= 0.0F ||
      !std::isfinite(tuning.maxSideFrictionBlendCoef) ||
      tuning.maxSideFrictionBlendCoef < 0.0F ||
      tuning.maxSideFrictionBlendCoef > 1.0F ||
      !std::isfinite(tuning.maxSideFrictionSliding) ||
      tuning.maxSideFrictionSliding < 0.0F ||
      !std::isfinite(tuning.m6FrictionModulationWhenSlipAndBrake) ||
      tuning.m6FrictionModulationWhenSlipAndBrake < 0.0F ||
      !std::isfinite(tuning.steerGroundTorqueSlippingCoef) ||
      tuning.steerGroundTorqueSlippingCoef < 0.0F ||
      !std::isfinite(tuning.m6InertialMass) || tuning.m6InertialMass <= 0.0F ||
      !std::isfinite(tuning.m6ForceEpsilon) || tuning.m6ForceEpsilon < 0.0F ||
      !std::isfinite(tuning.m6InertialTorqueModulationX) ||
      !std::isfinite(tuning.m6InertialTorqueModulationZ) ||
      !validCurve(tuning.m6RolloverLateralFromSpeedRatio) ||
      !validCurve(tuning.m6BurnoutRolloverFromSpeed) ||
      !validCurve(tuning.m6DonutRolloverFromSpeed) ||
      !validCurve(tuning.maxSideFriction) ||
      !validCurve(tuning.rolloverLateral) ||
      !validCurve(tuning.steerSlowDown) ||
      !std::isfinite(tuning.steerSlowDownCoef) || tuning.steerSlowDownCoef < 0.0F ||
      !validCurve(tuning.steerDriveTorque) ||
      !validCurve(tuning.modulationFromWheelCompression) ||
      !std::isfinite(tuning.brakeMaxDynamic) ||
      tuning.brakeMaxDynamic <= 0.0F || !std::isfinite(tuning.groundSlowDownCoef) ||
      tuning.groundSlowDownCoef < 0.0F || !validCurve(tuning.acceleration) ||
      !validCurve(tuning.reverseAcceleration) || !validDrivetrain(tuning)) {
    return {"vehicle tuning is invalid"};
  }
  tuning_ = tuning;
  centerOfMass_ = vehicleCenterOfMass(tuning_);
  inverseInertia_ = vehicleInverseInertia(tuning_);
  bodyEllipsoids_.clear();
  if (vehicleSolid != nullptr) {
    for (const gbx::SolidCollisionShape& shape : vehicleSolid->collisionShapes) {
      if (!shape.treeName.starts_with("BodySurf")) {
        continue;
      }
      const bool finiteTransform = std::ranges::all_of(
          shape.transform, [](const float value) { return std::isfinite(value); });
      if (shape.type != gbx::SolidCollisionType::Ellipsoid || !finiteTransform ||
          !std::isfinite(shape.minimum.x) || !std::isfinite(shape.minimum.y) ||
          !std::isfinite(shape.minimum.z) || !std::isfinite(shape.maximum.x) ||
          !std::isfinite(shape.maximum.y) || !std::isfinite(shape.maximum.z) ||
          shape.maximum.x <= 0.0F || shape.maximum.y <= 0.0F ||
          shape.maximum.z <= 0.0F ||
          std::abs(shape.minimum.x + shape.maximum.x) > 1.0e-4F ||
          std::abs(shape.minimum.y + shape.maximum.y) > 1.0e-4F ||
          std::abs(shape.minimum.z + shape.maximum.z) > 1.0e-4F) {
        return {"StadiumCar body collision ellipsoid is invalid"};
      }
      bodyEllipsoids_.push_back({
          {shape.transform[9], shape.transform[10], shape.transform[11]},
          shape.maximum,
          {shape.transform[0], shape.transform[1], shape.transform[2],
           shape.transform[3], shape.transform[4], shape.transform[5],
           shape.transform[6], shape.transform[7], shape.transform[8]}});
    }
    if (bodyEllipsoids_.size() != 4U) {
      return {"StadiumCar does not contain four body collision ellipsoids"};
    }
  }
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
    respawn.gear = 1;
    // The recovered BlockInfo Iso4 already identifies the respawn road plane.
    // Only reconcile downward; an overhead checkpoint mesh is not valid ground.
    const std::optional<content::TrackGroundHit> respawnGround = content::sampleTrackGround(
        collision, position.x, position.z, position.y - 16.0F,
        position.y + groundClearance_);
    if (respawnGround.has_value()) {
      respawn.position.y = respawnGround->height + groundClearance_;
      respawn.groundNormal = respawnGround->normal;
      respawn.grounded = true;
    }
    resetVehicleDynamics(respawn, tuning_);
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
  spawnState_.gear = 1;
  spawnState_.grounded = true;
  resetVehicleDynamics(spawnState_, tuning_);
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
  stepAccumulatorSeconds_ = 0.0;
}

float VehicleSimulation::frontWheelSteeringRadians() const noexcept {
  if (!validCurve(tuning_.visualSteerAngleFromSpeed)) {
    return 0.0F;
  }
  // FUN_009DCE00 samples the authored degree curve with absolute local
  // forward speed converted to km/h, then applies the smoothed tire steering.
  const float degrees = sampleCurve(tuning_.visualSteerAngleFromSpeed,
                                    std::abs(state_.speed) * 3.6F);
  return state_.smoothedSteering * degrees * kDegreesToRadians;
}

bool VehicleSimulation::restore(VehicleState state) noexcept {
  const bool validOrientation = std::ranges::all_of(
      state.orientationWxyz, [](const float value) { return std::isfinite(value); });
  const float orientationLengthSquared =
      state.orientationWxyz[0] * state.orientationWxyz[0] +
      state.orientationWxyz[1] * state.orientationWxyz[1] +
      state.orientationWxyz[2] * state.orientationWxyz[2] +
      state.orientationWxyz[3] * state.orientationWxyz[3];
  const bool validWheels = std::ranges::all_of(state.wheels, [&](const VehicleWheelState& wheel) {
    const float normalLengthSquared = dot(wheel.contactNormal, wheel.contactNormal);
    return std::isfinite(wheel.suspensionLength) && std::isfinite(wheel.suspensionSpeed) &&
           std::isfinite(wheel.pendingCorrection) &&
           std::isfinite(wheel.contactNormal.x) &&
           std::isfinite(wheel.contactNormal.y) &&
           std::isfinite(wheel.contactNormal.z) &&
           std::isfinite(wheel.contactPoint.x) &&
           std::isfinite(wheel.contactPoint.y) &&
           std::isfinite(wheel.contactPoint.z) &&
           (!wheel.contact || (normalLengthSquared >= 0.999F && normalLengthSquared <= 1.001F)) &&
           (!wheel.slipping || wheel.contact) &&
           wheel.suspensionLength >= tuning_.suspensionMinimum - 1.0e-5F &&
           wheel.suspensionLength <= tuning_.suspensionMaximum + 1.0e-5F &&
           wheel.pendingCorrection >= 0.0F &&
           wheel.pendingCorrection <=
               std::max(0.0F, wheel.suspensionLength - tuning_.suspensionMinimum) + 1.0e-5F;
  });
  if (collision_ == nullptr || !std::isfinite(state.position.x) ||
      !std::isfinite(state.position.y) || !std::isfinite(state.position.z) ||
      !std::isfinite(state.groundNormal.x) || !std::isfinite(state.groundNormal.y) ||
      !std::isfinite(state.groundNormal.z) || dot(state.groundNormal, state.groundNormal) <= 1.0e-10F ||
      !validOrientation || orientationLengthSquared < 0.9999F ||
      orientationLengthSquared > 1.0001F || !std::isfinite(state.angularVelocity.x) ||
      !std::isfinite(state.angularVelocity.y) || !std::isfinite(state.angularVelocity.z) ||
      !std::isfinite(state.cachedLocalAngularVelocity.x) ||
      !std::isfinite(state.cachedLocalAngularVelocity.y) ||
      !std::isfinite(state.cachedLocalAngularVelocity.z) ||
      !std::isfinite(state.priorLocalForcePerInertialMass.x) ||
      !std::isfinite(state.priorLocalForcePerInertialMass.y) ||
      !std::isfinite(state.priorLocalForcePerInertialMass.z) ||
      !validWheels ||
      !std::isfinite(state.smoothedSteering) || std::abs(state.smoothedSteering) > 1.0F ||
      !std::isfinite(state.horizontalVelocityX) ||
      !std::isfinite(state.horizontalVelocityZ) ||
      !std::isfinite(state.heading) || !std::isfinite(state.speed) ||
      !std::isfinite(state.engineRpm) || !std::isfinite(state.engineTargetRpm) ||
      !std::isfinite(state.gearShiftSeconds) ||
      !std::isfinite(state.verticalVelocity) || !std::isfinite(state.elapsedSeconds) ||
      !std::isfinite(state.lastWheelContactSeconds) ||
      state.lastWheelContactSeconds < 0.0F ||
      state.lastWheelContactSeconds > state.elapsedSeconds + 1.0e-4F ||
      !std::isfinite(state.raceTimeSeconds) || state.raceTimeSeconds < 0.0F ||
      state.checkpointCount > checkpoints_.size() || state.completedLaps > lapTotal_ ||
      (!lapRace_ && state.completedLaps != 0U) ||
      (lapRace_ && !state.finished && state.completedLaps >= lapTotal_) ||
      (state.finished && (state.checkpointCount != checkpoints_.size() ||
                          (lapRace_ && state.completedLaps != lapTotal_))) ||
      state.speed > tuning_.maxSpeed || state.speed < -tuning_.reverseMaxSpeed ||
      std::abs(state.engineRpm) > tuning_.m6MaxRpm || state.engineTargetRpm < 0.0F ||
      state.engineTargetRpm > tuning_.m6MaxRpm || state.gearShiftSeconds < 0.0F ||
      state.gearShiftSeconds > kGearShiftSeconds ||
      std::abs(state.verticalVelocity) > 500.0F) {
    return false;
  }
  syncPoseFromOrientation(state);
  const gbx::Float3 localVelocity = inverseRotate(
      rotationMatrix(state.orientationWxyz),
      {state.horizontalVelocityX, state.verticalVelocity, state.horizontalVelocityZ});
  if (std::abs(localVelocity.z - state.speed) > 1.0e-3F) {
    return false;
  }
  const std::int32_t lastForwardGear =
      static_cast<std::int32_t>(tuning_.m6GearRatio.size() - 1U);
  if ((state.gear != -1 && (state.gear < 1 || state.gear > lastForwardGear)) ||
      (state.gear < 0 && state.engineRpm > 0.0F) ||
      (state.gear > 0 && state.engineRpm < 0.0F)) {
    return false;
  }
  const bool hasWheelContact =
      std::ranges::any_of(state.wheels, [](const VehicleWheelState& wheel) { return wheel.contact; });
  if (!state.grounded) {
    for (VehicleWheelState& wheel : state.wheels) {
      wheel.contact = false;
      wheel.slipping = false;
    }
  } else if (!hasWheelContact) {
    const RotationMatrix rotation = rotationMatrix(state.orientationWxyz);
    for (std::size_t index = 0; index < state.wheels.size(); ++index) {
      VehicleWheelState& wheel = state.wheels[index];
      wheel.contact = true;
      wheel.contactNormal = {0.0F, 1.0F, 0.0F};
      wheel.contactPoint = add(
          state.position,
          rotate(rotation,
                 subtract(kWheelCenters[index],
                          {0.0F, wheel.suspensionLength + kWheelHalfAxes.y, 0.0F})));
      wheel.slipping = false;
    }
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
  stepAccumulatorSeconds_ = 0.0;
  return true;
}

void VehicleSimulation::update(const float deltaSeconds, VehicleControl control) noexcept {
  if (collision_ == nullptr || !std::isfinite(deltaSeconds) || deltaSeconds <= 0.0F) {
    return;
  }
  control.steering = std::clamp(control.steering, -1.0F, 1.0F);
  control.throttle = clampControl(control.throttle);
  control.brake = clampControl(control.brake);
  // A delayed suspension correction must not be divided by an arbitrary
  // microsecond-sized frame tail on the next call.
  constexpr double kFixedStepSeconds = 1.0 / 120.0;
  stepAccumulatorSeconds_ += static_cast<double>(std::min(deltaSeconds, 0.1F));
  while (stepAccumulatorSeconds_ >= kFixedStepSeconds) {
    stepAccumulatorSeconds_ -= kFixedStepSeconds;
    step(static_cast<float>(kFixedStepSeconds), control);
  }
}

void VehicleSimulation::step(const float deltaSeconds, const VehicleControl& control) noexcept {
  constexpr float kMetersPerSecondToKilometersPerHour = 3.6F;
  const bool hadGroundContact = state_.grounded;
  const float gravity =
      kStadiumGravity * (hadGroundContact ? tuning_.gravityCoef : tuning_.gravityCoefAir);
  const gbx::Float3 previousPosition = state_.position;
  state_.smoothedSteering += std::clamp(
      control.steering - state_.smoothedSteering, -tuning_.steerSpeed * deltaSeconds,
      tuning_.steerSpeed * deltaSeconds);
  const RotationMatrix controlRotation = rotationMatrix(state_.orientationWxyz);
  const gbx::Float3 velocityBeforeControl{state_.horizontalVelocityX, state_.verticalVelocity,
                                          state_.horizontalVelocityZ};
  const float previousSpeed = inverseRotate(controlRotation, velocityBeforeControl).z;
  float targetSpeed = previousSpeed;

  if (hadGroundContact) {
    if (control.throttle > 0.0F) {
      const float acceleration =
          std::max(0.0F, sampleCurve(tuning_.acceleration,
                                     targetSpeed * kMetersPerSecondToKilometersPerHour)) /
          tuning_.mass;
      targetSpeed += control.throttle * acceleration * deltaSeconds;
    }
    if (control.brake > 0.0F) {
      const float acceleration =
          targetSpeed > 0.1F
              ? tuning_.brakeMaxDynamic
              : std::max(0.0F, sampleCurve(tuning_.reverseAcceleration,
                                           std::min(0.0F, targetSpeed) *
                                               kMetersPerSecondToKilometersPerHour)) /
                    tuning_.mass;
      targetSpeed -= control.brake * acceleration * deltaSeconds;
    }
    if (control.throttle <= 0.001F && control.brake <= 0.001F) {
      targetSpeed = approachZero(targetSpeed, tuning_.groundSlowDownCoef * deltaSeconds);
    }
    targetSpeed = std::clamp(targetSpeed, -tuning_.reverseMaxSpeed, tuning_.maxSpeed);
    state_.speed = targetSpeed;

  }
  updateDrivetrain(deltaSeconds, control);

  const RotationMatrix beforeIntegration = rotationMatrix(state_.orientationWxyz);
  const gbx::Float3 oldVelocity{state_.horizontalVelocityX, state_.verticalVelocity,
                                state_.horizontalVelocityZ};
  gbx::Float3 force{0.0F, -gravity * tuning_.mass, 0.0F};
  if (hadGroundContact) {
    float longitudinalForce =
        (targetSpeed - previousSpeed) * tuning_.mass / deltaSeconds;
    const float driveDirection = previousSpeed < 0.0F ? -1.0F : 1.0F;
    longitudinalForce -=
        driveDirection * tuning_.steerSlowDownCoef * std::abs(state_.smoothedSteering) *
        sampleCurve(tuning_.steerSlowDown,
                    previousSpeed * kMetersPerSecondToKilometersPerHour);
    force = add(force, rotate(beforeIntegration, {0.0F, 0.0F, longitudinalForce}));
  }
  if (!hadGroundContact) {
    force = add(force, multiply(oldVelocity, -tuning_.linearFluidFrictionCoef));
  }
  const gbx::Float3 localVelocityForTires = inverseRotate(beforeIntegration, oldVelocity);
  const gbx::Float3 localAngularVelocityForTires =
      inverseRotate(beforeIntegration, state_.angularVelocity);
  const gbx::Float3 localForceBeforeTires = inverseRotate(beforeIntegration, force);
  const gbx::Float3 priorLocalForce =
      multiply(state_.priorLocalForcePerInertialMass, tuning_.m6InertialMass);
  const bool hasPriorLocalForce =
      dot(priorLocalForce, priorLocalForce) >=
      tuning_.m6ForceEpsilon * tuning_.m6ForceEpsilon;
  const gbx::Float3 centerOfMassBeforeIntegration =
      add(state_.position, rotate(beforeIntegration, centerOfMass_));
  const float forceLengthSquared = dot(localForceBeforeTires, localForceBeforeTires);
  const float localNormalForceRatio =
      forceLengthSquared <= 1.0e-10F
          ? 1.0F
          : std::abs(localForceBeforeTires.y) / std::sqrt(forceLengthSquared);
  const float lateralSlopeAdherence =
      adherenceFromRatio(localNormalForceRatio, tuning_.lateralSlopeAdherenceMin,
                         tuning_.lateralSlopeAdherenceMax);
  gbx::Float3 torque{};
  for (std::size_t index = 0; index < state_.wheels.size(); ++index) {
    VehicleWheelState& wheel = state_.wheels[index];
    const bool hadWheelContact = wheel.contact;
    const bool wasSlipping = wheel.slipping;
    const float oldLength = wheel.suspensionLength;
    const float absorbedLength = oldLength - wheel.pendingCorrection;
    wheel.suspensionLength =
        absorbedLength +
        tuning_.absorbTension * deltaSeconds * (tuning_.suspensionRest - absorbedLength);
    wheel.suspensionSpeed = (wheel.suspensionLength - oldLength) / deltaSeconds;
    wheel.pendingCorrection = 0.0F;
    if (!hadWheelContact) {
      continue;
    }
    const float suspensionForce =
        (tuning_.suspensionRest - wheel.suspensionLength) * tuning_.suspensionSpring -
        tuning_.suspensionDamping * wheel.suspensionSpeed;
    const gbx::Float3 worldForce =
        rotate(beforeIntegration, {0.0F, suspensionForce, 0.0F});
    force = add(force, worldForce);
    torque = add(torque,
                 cross(rotate(beforeIntegration, subtract(kWheelCenters[index], centerOfMass_)),
                       worldForce));
    if (hasPriorLocalForce) {
      // Model 5 uses the prior contact lever and stored force in body space,
      // then submits the X/Z result through the local-torque path.
      const gbx::Float3 localContactLever = inverseRotate(
          beforeIntegration, subtract(wheel.contactPoint, centerOfMassBeforeIntegration));
      const gbx::Float3 inertialTorque =
          cross(localContactLever, priorLocalForce);
      torque = add(
          torque,
          rotate(beforeIntegration,
                 {inertialTorque.x * tuning_.m6InertialTorqueModulationX, 0.0F,
                   inertialTorque.z * tuning_.m6InertialTorqueModulationZ}));
    }
    gbx::Float3 tireTangent{wheel.contactNormal.y, -wheel.contactNormal.x, 0.0F};
    const float tangentLengthSquared = dot(tireTangent, tireTangent);
    tireTangent = tangentLengthSquared <= 1.0e-10F
                      ? gbx::Float3{1.0F, 0.0F, 0.0F}
                      : multiply(tireTangent, 1.0F / std::sqrt(tangentLengthSquared));
    if (index < 2U) {
      const float steeringRadius =
          tuning_.steerRadiusMin +
          tuning_.steerRadiusCoef * std::abs(localVelocityForTires.z);
      // The recovered input scalar is positive-right; the portable control API is
      // positive-left.
      const float sourceSteering = -state_.smoothedSteering;
      const float steeringAngle =
          -sourceSteering *
          std::asin(std::clamp(1.0F / steeringRadius, -1.0F, 1.0F));
      tireTangent = add(multiply(tireTangent, std::cos(steeringAngle)),
                        {0.0F, 0.0F, -std::sin(steeringAngle)});
    }
    const float rawSideForce =
        -0.5F * tuning_.sideFriction1 * dot(localVelocityForTires, tireTangent);
    const float compressionCoordinate =
        (wheel.suspensionLength - tuning_.suspensionMaximum) /
        (tuning_.suspensionMinimum - tuning_.suspensionMaximum);
    const float priorSlipFactor = wasSlipping ? tuning_.maxSideFrictionSliding : 1.0F;
    const float brakeSlipFactor =
        wasSlipping && control.brake > 0.1F
            ? tuning_.m6FrictionModulationWhenSlipAndBrake
            : 1.0F;
    // Both Stadium vehicle-material records (Concrete 0 and Asphalt 16) store
    // the source multiplier 1.0, so no runtime material branch is needed here.
    const float sideForceCap =
        lateralSlopeAdherence *
        sampleCurve(tuning_.maxSideFriction,
                    localVelocityForTires.z * kMetersPerSecondToKilometersPerHour) *
        priorSlipFactor * brakeSlipFactor *
        sampleCurve(tuning_.modulationFromWheelCompression, compressionCoordinate);
    float sideForce = rawSideForce;
    if (std::abs(rawSideForce) <= sideForceCap) {
      wheel.slipping = false;
    } else {
      wheel.slipping = true;
      sideForce = std::lerp(std::copysign(sideForceCap, rawSideForce), rawSideForce,
                            tuning_.maxSideFrictionBlendCoef);
    }
    force = add(force, rotate(beforeIntegration, multiply(tireTangent, sideForce)));
  }

  if (hadGroundContact) {
    // +0x5C4 is a persistent direction latch. These are the exact ordered
    // ordinary-driving rules; burnout and active turbo override it in their
    // separate state machines, which the portable solver does not yet model.
    if (control.brake > 0.1F && localVelocityForTires.z < 10.0F &&
        std::abs(localVelocityForTires.x) < 2.0F) {
      state_.reverseDirectionLatched = true;
    }
    if (control.throttle > 0.1F &&
        (localVelocityForTires.z > 0.0F ||
         std::abs(localVelocityForTires.x) > 2.0F)) {
      state_.reverseDirectionLatched = false;
    }
    if (control.throttle < 0.1F && control.brake < 0.1F) {
      state_.reverseDirectionLatched = localVelocityForTires.z <= -2.0F;
    }
    // +0x840 stores the full physics-wheel Z span; 009DD380 multiplies it by
    // the exact 0.5 constant at 00D9E7A8 before selecting the axle sign.
    const float halfWheelbase =
        (kWheelCenters[0].z - kWheelCenters[2].z) * 0.5F;
    const float localSpeed = std::sqrt(dot(localVelocityForTires, localVelocityForTires));
    constexpr float kSteeringRampMinimumSpeed = 0.7F;
    constexpr float kHalfPi = 1.57079632679489661923F;
    const float lowSpeedFactor =
        localSpeed < kSteeringRampMinimumSpeed
            ? 0.0F
            : (localSpeed < tuning_.steerLowSpeed
                   ? std::sin(localSpeed / tuning_.steerLowSpeed * kHalfPi)
                   : 1.0F);
    for (std::size_t index = 0; index < state_.wheels.size(); ++index) {
      const VehicleWheelState& wheel = state_.wheels[index];
      const float axleOffset = index < 2U ? halfWheelbase : -halfWheelbase;
      const float rawYawForce =
          -0.5F * tuning_.sideFriction1 *
          (localVelocityForTires.x + localAngularVelocityForTires.y * axleOffset);
      // FUN_009DD380's later axle pass uses no slope-adherence output. The
      // Stadium material +0x20 multiplier is exactly 1.0 for both records.
      const float yawForceCap =
          sampleCurve(tuning_.maxSideFriction,
                      localVelocityForTires.z * kMetersPerSecondToKilometersPerHour);
      const bool yawSlipping = std::abs(rawYawForce) > yawForceCap;
      const float limitedYawForce =
          yawSlipping
              ? std::lerp(std::copysign(yawForceCap, rawYawForce), rawYawForce,
                          tuning_.maxSideFrictionBlendCoef)
              : rawYawForce;
      float wheelYawForce = tuning_.steerGroundTorque * limitedYawForce;
      if (index < 2U) {
        const float direction = state_.reverseDirectionLatched ? -1.0F : 1.0F;
        const float sourceSteering = -state_.smoothedSteering;
        wheelYawForce -=
            direction * lowSpeedFactor * sourceSteering *
            sampleCurve(tuning_.steerDriveTorque,
                        localVelocityForTires.z * kMetersPerSecondToKilometersPerHour) *
            (wheel.slipping ? tuning_.steerGroundTorqueSlippingCoef : 1.0F);
      }
      torque = add(torque,
                   rotate(beforeIntegration, {0.0F, wheelYawForce * axleOffset, 0.0F}));
    }
  }
  const float lateralSpeedRatio =
      localVelocityForTires.x * localVelocityForTires.x /
      (std::abs(localVelocityForTires.z) + 1.0F);
  const float rolloverTorque =
      sampleCurve(tuning_.m6RolloverLateralFromSpeedRatio,
                  lateralSpeedRatio * kMetersPerSecondToKilometersPerHour) *
      (localVelocityForTires.x <= 0.0F ? 1.0F : -1.0F);
  torque = add(torque, rotate(beforeIntegration, {0.0F, 0.0F, rolloverTorque}));
  state_.priorLocalForcePerInertialMass = multiply(
      inverseRotate(beforeIntegration, force), 1.0F / tuning_.m6InertialMass);
  const bool bodyCollisionPulse = state_.bodyCollisionPulse;
  state_.bodyCollisionPulse = false;
  gbx::Float3 angularDrag = multiply(localAngularVelocityForTires, -1.0F);
  if (hadGroundContact) {
    state_.lastWheelContactSeconds = state_.elapsedSeconds;
    state_.cachedLocalAngularVelocity = localAngularVelocityForTires;
  } else {
    if (bodyCollisionPulse) {
      // +0x5D4 refreshes the cached velocity but deliberately not +0x614.
      state_.cachedLocalAngularVelocity = localAngularVelocityForTires;
    } else if (state_.elapsedSeconds - state_.lastWheelContactSeconds <
               static_cast<float>(tuning_.airControlDuration) * 0.001F) {
      constexpr float kAirControlEpsilon = 1.0e-5F;
      gbx::Float3 adjustedAngularVelocity = localAngularVelocityForTires;
      const float steering = state_.smoothedSteering;
      const float oldYaw = state_.cachedLocalAngularVelocity.y;
      const bool oppositeYaw =
          (steering > kAirControlEpsilon && oldYaw < 0.0F) ||
          (steering < -kAirControlEpsilon && oldYaw > 0.0F);
      bool boostDrag = false;
      if (oppositeYaw) {
        if (std::abs(localAngularVelocityForTires.y) >
            tuning_.maxAngularSpeedYAirControl) {
          boostDrag = true;
          state_.cachedLocalAngularVelocity.y = localAngularVelocityForTires.y;
        }
      } else {
        boostDrag =
            (steering > kAirControlEpsilon && oldYaw > 0.0F) ||
            (steering < -kAirControlEpsilon && oldYaw < 0.0F);
        state_.cachedLocalAngularVelocity.y = localAngularVelocityForTires.y;
      }
      adjustedAngularVelocity.y = state_.cachedLocalAngularVelocity.y;
      state_.cachedLocalAngularVelocity.x =
          control.brake > kAirControlEpsilon &&
                  state_.cachedLocalAngularVelocity.x > 0.0F
              ? 0.0F
              : localAngularVelocityForTires.x;
      adjustedAngularVelocity.x = state_.cachedLocalAngularVelocity.x;
      if (boostDrag) {
        angularDrag = multiply(angularDrag, 3.0F);
      }
      angularDrag.z *= sampleCurve(
          tuning_.airControlZCoefFromAngularSpeed,
          std::abs(localAngularVelocityForTires.z));
      state_.angularVelocity = rotate(beforeIntegration, adjustedAngularVelocity);
    }
    const float angularDragSpeed = std::sqrt(dot(angularDrag, angularDrag));
    if (angularDragSpeed >= 1.0e-5F) {
      const float magnitude =
          tuning_.angularFluidFrictionCoef1 * angularDragSpeed +
          tuning_.angularFluidFrictionCoef2 * angularDragSpeed * angularDragSpeed;
      torque = add(
          torque,
          rotate(beforeIntegration,
                 multiply(angularDrag, magnitude / angularDragSpeed)));
    }
  }

  const gbx::Float3 preIntegratedPosition =
      add(state_.position, multiply(oldVelocity, deltaSeconds));
  const gbx::Float3 newVelocity =
      add(oldVelocity, multiply(force, deltaSeconds / tuning_.mass));
  const std::array<float, 4> angularQuaternion{
      0.0F, state_.angularVelocity.x, state_.angularVelocity.y, state_.angularVelocity.z};
  const std::array<float, 4> orientationDerivative =
      quaternionMultiply(angularQuaternion, state_.orientationWxyz);
  for (std::size_t component = 0; component < state_.orientationWxyz.size(); ++component) {
    state_.orientationWxyz[component] +=
        0.5F * orientationDerivative[component] * deltaSeconds;
  }
  state_.orientationWxyz = normalizeQuaternion(state_.orientationWxyz);
  const RotationMatrix afterIntegration = rotationMatrix(state_.orientationWxyz);
  state_.position = subtract(
      preIntegratedPosition,
      subtract(rotate(afterIntegration, centerOfMass_),
               rotate(beforeIntegration, centerOfMass_)));
  state_.horizontalVelocityX = newVelocity.x;
  state_.verticalVelocity = newVelocity.y;
  state_.horizontalVelocityZ = newVelocity.z;
  const gbx::Float3 localTorque = inverseRotate(beforeIntegration, torque);
  const gbx::Float3 angularAcceleration = rotate(
      beforeIntegration, {localTorque.x * inverseInertia_.x, localTorque.y * inverseInertia_.y,
                          localTorque.z * inverseInertia_.z});
  state_.angularVelocity =
      add(state_.angularVelocity, multiply(angularAcceleration, deltaSeconds));
  syncPoseFromOrientation(state_);
  const gbx::Float3 contactPosition = state_.position;
  gbx::Float3 contactVelocity{state_.horizontalVelocityX, state_.verticalVelocity,
                              state_.horizontalVelocityZ};
  const gbx::Float3 contactCenterOfMass =
      add(contactPosition, rotate(afterIntegration, centerOfMass_));
  std::vector<content::TrackEllipsoidContact> bodyContacts;
  for (const BodyEllipsoid& body : bodyEllipsoids_) {
    const gbx::Float3 bodyCenter =
        add(contactPosition, rotate(afterIntegration, body.center));
    const RotationMatrix bodyRotation =
        multiplyRotation(afterIntegration, body.rotation);
    content::appendTrackEllipsoidContacts(*collision_, bodyCenter, body.halfAxes,
                                          bodyRotation, bodyContacts);
  }
  for (content::TrackEllipsoidContact& contact : bodyContacts) {
    // 009DC910 zeroes both the correction and generic-response flag for these
    // two surface IDs before the engine queues the callback record.
    if (contact.surfaceId == 13 || contact.surfaceId == 23) {
      contact.correction = {};
      continue;
    }
    state_.bodyCollisionPulse = true;
    const gbx::Float3 lever = subtract(contact.point, contactCenterOfMass);
    const gbx::Float3 pointVelocity =
        add(contactVelocity, cross(state_.angularVelocity, lever));
    const float normalVelocity = dot(contact.normal, pointVelocity);
    if (normalVelocity >= 0.0F) {
      continue;
    }
    const gbx::Float3 normalPart = multiply(contact.normal, normalVelocity);
    gbx::Float3 tangentPart = subtract(pointVelocity, normalPart);
    const float tangentLengthSquared = dot(tangentPart, tangentPart);
    const float friction = contact.surfaceId == 4
                               ? tuning_.bodyFrictionCoefMetal
                               : tuning_.bodyFrictionCoefConcrete;
    const float tangentLimit = friction * std::abs(normalVelocity);
    if (tangentLengthSquared > tangentLimit * tangentLimit) {
      tangentPart = tangentLimit > 0.0F
                        ? multiply(tangentPart,
                                   tangentLimit / std::sqrt(tangentLengthSquared))
                        : gbx::Float3{};
    }
    const gbx::Float3 impulseDirection =
        normalize(multiply(add(normalPart, tangentPart), -1.0F));
    const gbx::Float3 leverCrossDirection = cross(lever, impulseDirection);
    const gbx::Float3 localAngularResponse =
        inverseRotate(afterIntegration, leverCrossDirection);
    const gbx::Float3 worldAngularResponse = rotate(
        afterIntegration,
        {localAngularResponse.x * inverseInertia_.x,
         localAngularResponse.y * inverseInertia_.y,
         localAngularResponse.z * inverseInertia_.z});
    const float effectiveInverseMass =
        1.0F / tuning_.mass +
        dot(impulseDirection, cross(worldAngularResponse, lever));
    if (std::abs(effectiveInverseMass) <= 1.0e-10F) {
      continue;
    }
    const float restitution = -(contact.surfaceId == 4
                                    ? tuning_.bodyRestCoefMetal
                                    : tuning_.bodyRestCoefConcrete);
    const float impulseMagnitude =
        (restitution - 1.0F) * dot(pointVelocity, impulseDirection) /
        effectiveInverseMass;
    applyRigidBodyImpulse(contactVelocity, state_.angularVelocity,
                          multiply(impulseDirection, impulseMagnitude), lever,
                          afterIntegration, inverseInertia_, tuning_);
  }
  state_.position = add(state_.position, reduceContactCorrections(bodyContacts));
  const gbx::Float3 suspensionAxis = rotate(afterIntegration, {0.0F, 1.0F, 0.0F});
  std::array<gbx::Float3, 4> activeCenters;
  std::array<std::optional<content::TrackEllipsoidContact>, 4> wheelContacts;
  for (std::size_t index = 0; index < state_.wheels.size(); ++index) {
    activeCenters[index] = add(
        contactPosition,
        rotate(afterIntegration,
               subtract(kWheelCenters[index],
                        {0.0F, state_.wheels[index].suspensionLength, 0.0F})));
    wheelContacts[index] = content::sampleTrackEllipsoidContact(
        *collision_, activeCenters[index], kWheelHalfAxes, afterIntegration, suspensionAxis);
  }
  bool hasWheelContact = false;
  gbx::Float3 residualCorrection{};
  for (std::size_t index = 0; index < state_.wheels.size(); ++index) {
    VehicleWheelState& wheel = state_.wheels[index];
    const auto& contact = wheelContacts[index];
    if (!contact.has_value()) {
      wheel.contact = false;
      wheel.slipping = false;
      wheel.contactNormal = {0.0F, 1.0F, 0.0F};
      continue;
    }
    // 009DC910 rejects Water and GolfBall before the wheel callback, so they
    // provide neither suspension support nor correction/impulse response.
    if (contact->surfaceId == 13 || contact->surfaceId == 23) {
      wheel.contact = false;
      wheel.slipping = false;
      wheel.contactNormal = {0.0F, 1.0F, 0.0F};
      continue;
    }
    constexpr float kMaximumTireLocalNormalX = 0.707106781F;
    const gbx::Float3 localContactNormal =
        normalize(inverseRotate(afterIntegration, contact->normal));
    wheel.contactPoint = contact->point;
    const bool supportsTire = std::abs(localContactNormal.x) < kMaximumTireLocalNormalX;
    wheel.contact = supportsTire;
    wheel.contactNormal = localContactNormal;
    if (!supportsTire) {
      wheel.slipping = false;
    }
    hasWheelContact = hasWheelContact || supportsTire;
    const float remainingTravel =
        std::max(0.0F, wheel.suspensionLength - tuning_.suspensionMinimum);
    const float absorbedCorrection = std::min(contact->axisCorrection, remainingTravel);
    wheel.pendingCorrection = std::max(wheel.pendingCorrection, absorbedCorrection);
    const gbx::Float3 residual =
        subtract(contact->correction, multiply(suspensionAxis, absorbedCorrection));
    residualCorrection = add(residualCorrection, residual);
    const gbx::Float3 contactLever = subtract(contact->point, contactCenterOfMass);
    gbx::Float3 impulseLever = contactLever;
    gbx::Float3 responseVelocity =
        add(contactVelocity, cross(state_.angularVelocity, contactLever));
    const float normalVelocity = dot(responseVelocity, contact->normal);
    const bool atTravelLimit = contact->axisCorrection <= 0.0F ||
                               remainingTravel <= contact->axisCorrection;
    const float suspensionNormalVelocity =
        normalVelocity * dot(contact->normal, suspensionAxis);
    if (supportsTire && !atTravelLimit && suspensionNormalVelocity < 0.0F) {
      impulseLever = subtract(activeCenters[index], contactCenterOfMass);
      responseVelocity = subtract(
          responseVelocity, multiply(suspensionAxis, suspensionNormalVelocity));
    }
    const float responseNormalVelocity = dot(responseVelocity, contact->normal);
    if (responseNormalVelocity < 0.0F) {
      const gbx::Float3 leverCrossNormal = cross(impulseLever, contact->normal);
      const gbx::Float3 localAngularResponse =
          inverseRotate(afterIntegration, leverCrossNormal);
      const gbx::Float3 worldAngularResponse = rotate(
          afterIntegration,
          {localAngularResponse.x * inverseInertia_.x,
           localAngularResponse.y * inverseInertia_.y,
           localAngularResponse.z * inverseInertia_.z});
      const float effectiveInverseMass =
          1.0F / tuning_.mass +
          dot(contact->normal, cross(worldAngularResponse, impulseLever));
      if (effectiveInverseMass > 1.0e-10F) {
        const float restitution = contact->surfaceId == 4
                                      ? tuning_.wheelRestCoefMetal
                                      : tuning_.wheelRestCoefConcrete;
        const gbx::Float3 impulse =
            multiply(contact->normal,
                     -(1.0F + restitution) * responseNormalVelocity /
                         effectiveInverseMass);
        applyRigidBodyImpulse(contactVelocity, state_.angularVelocity, impulse,
                              impulseLever, afterIntegration, inverseInertia_, tuning_);
      }
    }
  }
  state_.position = add(state_.position, residualCorrection);
  state_.horizontalVelocityX = contactVelocity.x;
  state_.verticalVelocity = contactVelocity.y;
  state_.horizontalVelocityZ = contactVelocity.z;
  state_.grounded = hasWheelContact;
  const gbx::Float3 localVelocity = inverseRotate(
      afterIntegration,
      {state_.horizontalVelocityX, state_.verticalVelocity, state_.horizontalVelocityZ});
  state_.speed = std::abs(localVelocity.z) < 0.05F ? 0.0F : localVelocity.z;
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

void VehicleSimulation::updateDrivetrain(const float deltaSeconds,
                                         const VehicleControl& control) noexcept {
  const std::int32_t lastForwardGear =
      static_cast<std::int32_t>(tuning_.m6GearRatio.size() - 1U);
  state_.gear = state_.gear == -1 || (state_.gear >= 1 && state_.gear <= lastForwardGear)
                    ? state_.gear
                    : 1;
  if (state_.gearShiftSeconds > 0.0F) {
    state_.gearShiftSeconds = std::max(0.0F, state_.gearShiftSeconds - deltaSeconds);
  }

  const float selectedAccelerator = state_.gear < 0 ? control.brake : control.throttle;
  const bool throttleActive = selectedAccelerator > 0.1F;
  const bool wantsReverse = state_.speed < -0.05F ||
                            (std::abs(state_.speed) <= 0.05F && control.brake > control.throttle);
  const bool wantsForward = state_.speed > 0.05F ||
                            (std::abs(state_.speed) <= 0.05F && control.throttle > control.brake);
  const std::int32_t desiredDirection = wantsReverse ? -1 : wantsForward ? 1 : state_.gear < 0 ? -1 : 1;
  const std::size_t ratioIndex = state_.gear < 0 ? 0U : static_cast<std::size_t>(state_.gear);
  float targetRpm = std::min(std::abs(state_.speed) * tuning_.m6GearRatio[ratioIndex],
                             tuning_.m6MaxRpm);
  if ((desiredDirection < 0) != (state_.gear < 0)) {
    targetRpm = 0.0F;
  }
  state_.engineTargetRpm = targetRpm;

  float rpm = std::abs(state_.engineRpm);
  const bool uncoupled = !state_.grounded || state_.gearShiftSeconds > 0.0F;
  if (uncoupled) {
    rpm += (throttleActive ? tuning_.m6BurnoutRpmAcceleration
                           : -tuning_.m6RpmLossOnTakeOffFinished) * deltaSeconds;
  } else if (targetRpm <= rpm) {
    // ponytail: the unrecovered tire-slip state selects air deadening here; use the
    // normal coupled branch until the exact contact solver exposes that state.
    const float loss = throttleActive ? tuning_.m6AirRpmDeadening
                                      : tuning_.m6RpmLossOnGearUp;
    rpm -= loss * deltaSeconds;
  } else {
    rpm += tuning_.m6AirRpmAcceleration * deltaSeconds;
  }

  if (!uncoupled) {
    if ((desiredDirection < 0) != (state_.gear < 0)) {
      if (rpm < kDirectionChangeRpm) {
        state_.gear = desiredDirection < 0 ? -1 : 1;
        state_.gearShiftSeconds = kGearShiftSeconds;
      }
    } else if (state_.gear > 0) {
      const std::size_t gear = static_cast<std::size_t>(state_.gear);
      if (state_.gear < lastForwardGear &&
          targetRpm > tuning_.m6MaxRpm * tuning_.m6MaxRpmByGear[gear]) {
        ++state_.gear;
        state_.gearShiftSeconds = kGearShiftSeconds;
      } else if (state_.gear > 1 &&
                 targetRpm < tuning_.m6MaxRpm * tuning_.m6MinRpmByGear[gear]) {
        --state_.gear;
        state_.gearShiftSeconds = kGearShiftSeconds;
      }
    }
  }
  rpm = std::clamp(rpm, 0.0F, tuning_.m6MaxRpm);
  state_.engineRpm = state_.gear < 0 ? -rpm : rpm;
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
