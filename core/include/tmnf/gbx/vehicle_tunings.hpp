#pragma once

#include "tmnf/gbx/input.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace tmnf::gbx {

struct RealCurve final {
  std::vector<float> x;
  std::vector<float> y;
  std::int32_t interpolation = 0;
};

struct VehicleTuning final {
  std::string name;
  RealCurve acceleration;
  RealCurve reverseAcceleration;
  float mass = 0.0F;
  float steerRadiusMin = 0.0F;
  float steerRadiusCoef = 0.0F;
  float steerSpeed = 0.0F;
  std::int32_t steerModel = -1;
  float steerGroundTorque = 0.0F;
  float maxSpeed = 0.0F;
  float suspensionSpring = 0.0F;
  float suspensionDamping = 0.0F;
  float suspensionMinimum = 0.0F;
  float suspensionMaximum = 0.0F;
  float suspensionRest = 0.0F;
  float relativeSpeedMultiplier = 0.0F;
  float absorbTension = 0.0F;
  float reverseMaxSpeed = 0.0F;
  float turboBoost = 0.0F;
  float turbo2Boost = 0.0F;
  std::int32_t turboDuration = 0;
  std::int32_t turbo2Duration = 0;
  RealCurve visualSteerAngleFromSpeed;
  float centerOfMassAftForce = 0.0F;
  float centerOfMassDownUp = 0.0F;
  float inertiaMass = 0.0F;
  std::array<float, 3> inertiaHalfDiagonal{};
  std::int32_t tireMaterial = -1;
  std::int32_t shockModel = -1;
  float gravityCoef = 0.0F;
  float gravityCoefAir = 0.0F;
  float linearFluidFrictionCoef = 0.0F;
  float angularFluidFrictionCoef1 = 0.0F;
  float angularFluidFrictionCoef2 = 0.0F;
  std::int32_t airControlDuration = 0;
  float maxAngularSpeedYAirControl = 0.0F;
  RealCurve airControlZCoefFromAngularSpeed;
  float bodyFrictionCoefConcrete = 0.0F;
  float bodyFrictionCoefMetal = 0.0F;
  float bodyRestCoefMetal = 0.0F;
  float bodyRestCoefConcrete = 0.0F;
  float wheelFrictionCoefConcrete = 0.0F;
  float wheelRestCoefConcrete = 0.0F;
  float wheelFrictionCoefMetal = 0.0F;
  float wheelRestCoefMetal = 0.0F;
  float angularSpeedYImpulseScale = 0.0F;
  float angularSpeedClamp = 0.0F;
  float linearSpeed2PositiveDeltaMax = 0.0F;
  float steerLowSpeed = 0.0F;
  float steerAngleMax = 0.0F;
  float slipAngleForceMax = 0.0F;
  float slipAngleForceCoef1 = 0.0F;
  float slipAngleForceCoef2 = 0.0F;
  float lateralSlopeAdherenceMin = 0.0F;
  float lateralSlopeAdherenceMax = 0.0F;
  float axialSlopeAdherenceMin = 0.0F;
  float axialSlopeAdherenceMax = 0.0F;
  float steerGroundTorqueSlippingCoef = 0.0F;
  float maxSideFrictionBlendCoef = 0.0F;
  RealCurve maxSideFriction;
  float maxSideFrictionSliding = 0.0F;
  float sideFriction1 = 0.0F;
  RealCurve rolloverLateral;
  RealCurve steerSlowDown;
  float steerSlowDownCoef = 0.0F;
  RealCurve steerDriveTorque;
  RealCurve modulationFromWheelCompression;
  float brakeMaxDynamic = 0.0F;
  float groundSlowDownCoef = 0.0F;
  float m6MaxRpm = 0.0F;
  std::vector<float> m6GearRatio;
  std::vector<float> m6MaxRpmByGear;
  std::vector<float> m6MinRpmByGear;
  std::vector<float> m6RpmWantedOnGearUp;
  float m6BurnoutRpmAcceleration = 0.0F;
  float m6AirRpmAcceleration = 0.0F;
  float m6AirRpmDeadening = 0.0F;
  float m6RpmLossOnGearUp = 0.0F;
  float m6RpmGainOnGearDown = 0.0F;
  float m6SpeedLimitPositiveForTakeOffFront = 0.0F;
  float m6SpeedLimitPositiveForTakeOffRear = 0.0F;
  float m6SpeedLimitNegativeForTakeOffFront = 0.0F;
  float m6SpeedLimitNegativeForTakeOffRear = 0.0F;
  float m6RpmGainOnTakeOff = 0.0F;
  float m6RpmLossOnTakeOffFinished = 0.0F;
  float m6FrictionModulationWhenSlipAndBrake = 0.0F;
  float m6InertialMass = 0.0F;
  float m6ForceEpsilon = 0.0F;
  float m6InertialTorqueModulationX = 0.0F;
  float m6InertialTorqueModulationZ = 0.0F;
  RealCurve m6RolloverLateralFromSpeedRatio;
  RealCurve m6BurnoutRolloverFromSpeed;
  RealCurve m6DonutRolloverFromSpeed;
  bool fakeEngine = false;
};

struct VehicleTunings final {
  std::vector<VehicleTuning> entries;
  std::int32_t activeIndex = -1;

  [[nodiscard]] const VehicleTuning* active() const noexcept {
    return activeIndex >= 0 && static_cast<std::size_t>(activeIndex) < entries.size()
               ? &entries[static_cast<std::size_t>(activeIndex)]
               : nullptr;
  }
};

struct VehicleTuningsParseResult final {
  VehicleTunings tunings;
  std::string error;

  [[nodiscard]] explicit operator bool() const noexcept { return error.empty(); }
};

// Parses the legacy TMNF CPlugVehiclePhyTunings stream, including the
// parser-timed dummy writes required by encrypted NadeoPak entries.
[[nodiscard]] VehicleTuningsParseResult parseVehicleTunings(Input& input);

} // namespace tmnf::gbx
