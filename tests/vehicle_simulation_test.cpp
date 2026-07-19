#include "tmnf/content/track_collision.hpp"
#include "tmnf/gameplay/vehicle.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <utility>

namespace {

void addTrigger(tmnf::content::TrackScene& scene, const std::size_t blockIndex,
                const float localZ) {
  tmnf::content::TrackTrigger trigger;
  trigger.blockIndex = blockIndex;
  trigger.logicalPath = "FixtureTrigger.Solid.Gbx";
  tmnf::gbx::SolidCollisionShape shape;
  shape.type = tmnf::gbx::SolidCollisionType::Mesh;
  shape.positions = {
      {0.0F, 0.0F, localZ}, {32.0F, 0.0F, localZ}, {32.0F, 4.0F, localZ}, {0.0F, 4.0F, localZ}};
  shape.indices = {0, 1, 2, 0, 2, 3};
  trigger.solid.collisionShapes.push_back(std::move(shape));
  scene.triggers.push_back(std::move(trigger));
}

tmnf::content::TrackScene makeScene() {
  tmnf::content::TrackScene scene;
  scene.challenge.size = {8, 8, 8};
  tmnf::gbx::ChallengeBlock start;
  start.name = "FixtureStartLine";
  start.coordinate = {1, 0, 1};
  scene.challenge.blocks.push_back(start);
  tmnf::gbx::BlockInfoMetadata startMetadata;
  startMetadata.waypointType = tmnf::gbx::BlockWaypointType::Start;
  startMetadata.spawnAir = std::array<float, 12>{1.0F, 0.0F, 0.0F, 0.0F,  1.0F, 0.0F,
                                                 0.0F, 0.0F, 1.0F, 20.0F, 0.3F, 12.0F};
  scene.blockDefinitions.push_back({"FixtureStartLine", startMetadata});

  tmnf::gbx::ChallengeBlock checkpoint;
  checkpoint.name = "FixtureCheckpoint";
  checkpoint.coordinate = {1, 0, 2};
  scene.challenge.blocks.push_back(checkpoint);
  scene.challenge.checkpoints.push_back({1, 0, 2});
  tmnf::gbx::BlockInfoMetadata checkpointMetadata;
  checkpointMetadata.waypointType = tmnf::gbx::BlockWaypointType::Checkpoint;
  checkpointMetadata.spawnAir = std::array<float, 12>{1.0F, 0.0F, 0.0F, 0.0F,  1.0F, 0.0F,
                                                      0.0F, 0.0F, 1.0F, 20.0F, 0.3F, 4.0F};
  scene.blockDefinitions.push_back({"FixtureCheckpoint", checkpointMetadata});

  tmnf::gbx::ChallengeBlock finish;
  finish.name = "FixtureFinishLine";
  finish.coordinate = {1, 0, 3};
  scene.challenge.blocks.push_back(finish);
  tmnf::gbx::BlockInfoMetadata finishMetadata;
  finishMetadata.waypointType = tmnf::gbx::BlockWaypointType::Finish;
  finishMetadata.spawnAir = checkpointMetadata.spawnAir;
  scene.blockDefinitions.push_back({"FixtureFinishLine", finishMetadata});
  addTrigger(scene, 1, 4.0F);
  addTrigger(scene, 2, 4.0F);

  tmnf::content::TrackModel model;
  tmnf::gbx::SolidCollisionShape shape;
  shape.type = tmnf::gbx::SolidCollisionType::Mesh;
  shape.archiveVersion = 3;
  shape.positions = {{-128.0F, 0.0F, -128.0F},
                     {128.0F, 0.0F, -128.0F},
                     {128.0F, 0.0F, 128.0F},
                     {-128.0F, 0.0F, 128.0F}};
  shape.indices = {0, 2, 1, 0, 3, 2};
  shape.triangleSurfaceIndices = {0, 0};
  model.solid.collisionShapes.push_back(std::move(shape));
  tmnf::gbx::SolidCollisionShape checkpointArch;
  checkpointArch.type = tmnf::gbx::SolidCollisionType::Mesh;
  checkpointArch.archiveVersion = 3;
  checkpointArch.positions = {
      {18.0F, 6.0F, 34.0F}, {22.0F, 6.0F, 34.0F},
      {22.0F, 6.0F, 38.0F}, {18.0F, 6.0F, 38.0F}};
  checkpointArch.indices = {0, 2, 1, 0, 3, 2};
  checkpointArch.triangleSurfaceIndices = {0, 0};
  model.solid.collisionShapes.push_back(std::move(checkpointArch));
  model.solid.surfaceMaterials.push_back(16);
  scene.models.push_back(std::move(model));
  scene.placements.push_back({0, 0});
  return scene;
}

tmnf::content::TrackScene makeWallScene() {
  tmnf::content::TrackScene scene = makeScene();
  tmnf::gbx::SolidCollisionShape wall;
  wall.type = tmnf::gbx::SolidCollisionType::Mesh;
  wall.archiveVersion = 3;
  wall.positions = {
      {-128.0F, 0.0F, 24.0F}, {128.0F, 0.0F, 24.0F}, {128.0F, 4.0F, 24.0F}, {-128.0F, 4.0F, 24.0F}};
  // 008C1C70 is one-sided; face the wall toward the approaching vehicle.
  wall.indices = {0, 2, 1, 0, 3, 2};
  wall.triangleSurfaceIndices = {0, 0};
  scene.models[0].solid.collisionShapes.push_back(std::move(wall));
  return scene;
}

tmnf::gbx::SolidModel makeVehicleCollisionSolid() {
  tmnf::gbx::SolidModel solid;
  const auto addBody = [&](std::string name, const tmnf::gbx::Float3 axes,
                           const std::array<float, 12>& transform) {
    tmnf::gbx::SolidCollisionShape shape;
    shape.treeName = std::move(name);
    shape.type = tmnf::gbx::SolidCollisionType::Ellipsoid;
    shape.transform = transform;
    shape.minimum = {-axes.x, -axes.y, -axes.z};
    shape.maximum = axes;
    shape.surfaceId = 0;
    solid.collisionShapes.push_back(std::move(shape));
  };
  addBody("BodySurf", {0.439118F, 0.362F, 1.90153F},
          {1.0F, 0.0F, 0.0F, 0.0F, 0.998223F, -0.059586F,
           0.0F, 0.059586F, 0.998223F, 0.0F, 0.471253F, 0.219106F});
  addBody("BodySurf01", {0.968297F, 0.362741F, 1.68228F},
          {1.0F, 0.0F, 0.0F, 0.0F, 0.998954F, -0.045716F,
           0.0F, 0.045716F, 0.998954F, 0.0F, 0.448782F, -0.20792F});
  addBody("BodySurf02", {1.02092F, 0.515218F, 1.03801F},
          {1.0F, 0.0F, 0.0F, 0.0F, 0.9989F, -0.046888F,
           0.0F, 0.046888F, 0.9989F, 0.0F, 0.652812F, -0.89763F});
  addBody("BodySurf04", {0.384841F, 0.905323F, 0.283418F},
          {0.0F, -1.0F, 0.0F, 0.0F, 0.0F, -1.0F,
           1.0F, 0.0F, 0.0F, -0.015532F, 0.363252F, 1.75357F});
  return solid;
}

tmnf::gbx::SolidModel makeImpulseCollisionSolid(const tmnf::gbx::Float3 centerOfMass,
                                                const float leverX) {
  tmnf::gbx::SolidModel solid;
  for (std::size_t index = 0; index < 4U; ++index) {
    tmnf::gbx::SolidCollisionShape shape;
    shape.treeName = index == 0U ? "BodySurf" : "BodySurf0" + std::to_string(index);
    shape.type = tmnf::gbx::SolidCollisionType::Ellipsoid;
    shape.minimum = {-0.25F, -0.25F, -0.5F};
    shape.maximum = {0.25F, 0.25F, 0.5F};
    if (index == 0U) {
      shape.transform[9] = centerOfMass.x + leverX;
      shape.transform[10] = centerOfMass.y;
      shape.transform[11] = centerOfMass.z - shape.maximum.z;
    } else {
      shape.transform[9] = 100.0F + static_cast<float>(index);
      shape.transform[10] = 100.0F;
      shape.transform[11] = 100.0F;
    }
    solid.collisionShapes.push_back(std::move(shape));
  }
  return solid;
}

tmnf::content::TrackScene makeWheelSupportScene() {
  tmnf::content::TrackScene scene = makeScene();
  auto& shapes = scene.models[0].solid.collisionShapes;
  shapes.clear();
  const auto addGround = [&](const float minimumX, const float maximumX,
                             const float minimumZ, const float maximumZ) {
    tmnf::gbx::SolidCollisionShape ground;
    ground.type = tmnf::gbx::SolidCollisionType::Mesh;
    ground.archiveVersion = 3;
    ground.positions = {{minimumX, 0.0F, minimumZ}, {maximumX, 0.0F, minimumZ},
                        {maximumX, 0.0F, maximumZ}, {minimumX, 0.0F, maximumZ}};
    ground.indices = {0, 2, 1, 0, 3, 2};
    ground.triangleSurfaceIndices = {0, 0};
    shapes.push_back(std::move(ground));
  };
  addGround(8.0F, 32.0F, 0.0F, 24.0F);
  addGround(48.85F, 48.88F, 49.77F, 49.80F);
  addGround(67.98F, 68.02F, 67.98F, 68.02F);
  return scene;
}

tmnf::gbx::VehicleTuning makeVehicleTuning() {
  tmnf::gbx::VehicleTuning tuning;
  tuning.name = "20juin2006";
  tuning.acceleration.x = {-1000.0F, -102.0F, -80.0F, -35.0F, 0.0F,
                           101.0F, 201.0F, 401.0F, 801.0F};
  tuning.acceleration.y = {1.0F, 2.0F, 6.0F, 12.0F, 16.0F, 11.0F, 7.0F, 5.5F, 1.0F};
  tuning.reverseAcceleration.x = {-150.0F, -100.0F, -50.0F, 0.0F};
  tuning.reverseAcceleration.y = {0.1F, 1.5F, 8.0F, 30.0F};
  tuning.mass = 1.0F;
  tuning.steerRadiusMin = 2.0F;
  tuning.steerRadiusCoef = 0.85F;
  tuning.steerSpeed = 20.0F;
  tuning.steerModel = 5;
  tuning.steerGroundTorque = 0.08F;
  tuning.visualSteerAngleFromSpeed.x = {0.0F, 100.0F};
  tuning.visualSteerAngleFromSpeed.y = {30.0F, 10.0F};
  tuning.maxSpeed = 277.7778F;
  tuning.suspensionSpring = 40.0F;
  tuning.suspensionDamping = 1.0F;
  tuning.suspensionMinimum = 0.0F;
  tuning.suspensionMaximum = 0.7F;
  tuning.suspensionRest = 0.2F;
  tuning.relativeSpeedMultiplier = 0.2F;
  tuning.absorbTension = 5.0F;
  tuning.reverseMaxSpeed = 138.61111F;
  tuning.centerOfMassAftForce = 0.0F;
  tuning.centerOfMassDownUp = 0.45F;
  tuning.inertiaMass = 5.0F;
  tuning.inertiaHalfDiagonal = {0.5F, 0.5F, 1.0F};
  tuning.tireMaterial = 9;
  tuning.shockModel = 2;
  tuning.gravityCoef = 3.0F;
  tuning.gravityCoefAir = 2.5F;
  tuning.linearFluidFrictionCoef = 0.03F;
  tuning.angularFluidFrictionCoef1 = 0.4F;
  tuning.angularFluidFrictionCoef2 = 0.2F;
  tuning.airControlDuration = 30'000;
  tuning.maxAngularSpeedYAirControl = 3.14F;
  tuning.airControlZCoefFromAngularSpeed.x = {0.0F};
  tuning.airControlZCoefFromAngularSpeed.y = {1.0F};
  tuning.bodyFrictionCoefConcrete = 0.3F;
  tuning.bodyFrictionCoefMetal = 0.2F;
  tuning.bodyRestCoefMetal = -0.5F;
  tuning.bodyRestCoefConcrete = -0.5F;
  tuning.wheelFrictionCoefConcrete = 0.0F;
  tuning.wheelRestCoefConcrete = -0.8F;
  tuning.wheelFrictionCoefMetal = 0.0F;
  tuning.wheelRestCoefMetal = -0.8F;
  tuning.angularSpeedYImpulseScale = 0.55F;
  tuning.angularSpeedClamp = 100.0F;
  tuning.linearSpeed2PositiveDeltaMax = 10'000.0F;
  tuning.steerLowSpeed = 30.0F;
  tuning.steerAngleMax = 0.523599F;
  tuning.lateralSlopeAdherenceMin = 0.3F;
  tuning.lateralSlopeAdherenceMax = 0.7F;
  tuning.axialSlopeAdherenceMin = 0.4F;
  tuning.axialSlopeAdherenceMax = 0.7F;
  tuning.steerGroundTorqueSlippingCoef = 1.0F;
  tuning.maxSideFrictionBlendCoef = 0.018F;
  tuning.maxSideFriction.x = {0.0F, 100.0F, 200.0F, 300.0F, 400.0F, 500.0F};
  tuning.maxSideFriction.y = {80.0F, 80.0F, 75.0F, 67.0F, 60.0F, 55.0F};
  tuning.maxSideFrictionSliding = 0.35F;
  tuning.sideFriction1 = 40.0F;
  tuning.rolloverLateral.x = {0.0F};
  tuning.rolloverLateral.y = {0.0F};
  tuning.steerSlowDown.x = {-50.0F, 0.0F, 50.0F, 100.0F, 200.0F, 300.0F};
  tuning.steerSlowDown.y = {1.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F};
  tuning.steerSlowDownCoef = 0.5F;
  tuning.steerDriveTorque.x = {-100.0F, 0.0F, 50.0F, 100.0F, 150.0F,
                               200.0F, 300.0F, 400.0F, 500.0F, 600.0F};
  tuning.steerDriveTorque.y = {7.0F, 16.0F, 10.4F, 8.5F, 6.8F,
                               5.5F, 4.25F, 4.0F, 3.75F, 3.75F};
  tuning.modulationFromWheelCompression.x = {0.0F, 0.6F, 0.975F, 1.0F};
  tuning.modulationFromWheelCompression.y = {0.1F, 0.3F, 1.0F, 1.0F};
  tuning.brakeMaxDynamic = 30.0F;
  tuning.groundSlowDownCoef = 0.3F;
  tuning.m6MaxRpm = 11'000.0F;
  tuning.m6GearRatio = {400.0F, 370.0F, 220.0F, 160.0F, 110.0F, 85.0F};
  tuning.m6MaxRpmByGear = {0.0F, 0.95F, 0.9F, 0.95F, 0.95F, 1.0F};
  tuning.m6MinRpmByGear = {0.0F, 0.0F, 0.5F, 0.57F, 0.55F, 0.6F};
  tuning.m6RpmWantedOnGearUp = {0.0F, 0.0F, 0.628F, 0.7F, 0.7F, 0.72F};
  tuning.m6BurnoutRpmAcceleration = 6'000.0F;
  tuning.m6AirRpmAcceleration = 6'000.0F;
  tuning.m6AirRpmDeadening = 3'000.0F;
  tuning.m6RpmLossOnGearUp = 17'000.0F;
  tuning.m6RpmGainOnGearDown = 11'500.0F;
  tuning.m6RpmGainOnTakeOff = 10'000.0F;
  tuning.m6RpmLossOnTakeOffFinished = 4'000.0F;
  tuning.m6FrictionModulationWhenSlipAndBrake = 0.2F;
  tuning.m6InertialMass = 5.0F;
  tuning.m6ForceEpsilon = 0.01F;
  tuning.m6InertialTorqueModulationX = 0.05F;
  tuning.m6InertialTorqueModulationZ = 0.05F;
  tuning.m6RolloverLateralFromSpeedRatio.x = {0.0F, 1.0F, 10.0F, 20.0F, 30.0F};
  tuning.m6RolloverLateralFromSpeedRatio.y = {0.0F, 0.8F, 1.0F, 1.0F, 1.0F};
  tuning.m6BurnoutRolloverFromSpeed.x = {0.0F, 10.0F, 50.0F};
  tuning.m6BurnoutRolloverFromSpeed.y = {1.5F, 1.2F, 0.2F};
  tuning.m6DonutRolloverFromSpeed.x = {0.0F, 10.0F, 20.0F};
  tuning.m6DonutRolloverFromSpeed.y = {0.0F, 5.0F, 6.0F};
  return tuning;
}

} // namespace

int main() {
  const tmnf::gbx::VehicleTuning tuning = makeVehicleTuning();
  const tmnf::gbx::SolidModel vehicleSolid = makeVehicleCollisionSolid();
  const tmnf::content::TrackScene scene = makeScene();
  const tmnf::content::TrackCollisionBuildResult collision =
      tmnf::content::buildTrackCollision(scene);
  if (!collision) {
    std::cerr << collision.error << '\n';
    return 1;
  }
  tmnf::gameplay::VehicleSimulation simulation;
  const tmnf::gameplay::VehicleInitializeResult initialized =
      simulation.initialize(scene, collision.collision, tuning);
  if (!initialized) {
    std::cerr << initialized.error << '\n';
    return 1;
  }
  const tmnf::gameplay::VehicleState spawn = simulation.state();
  if (!spawn.grounded || std::abs(spawn.position.x - 52.0F) > 1.0e-5F ||
      std::abs(spawn.position.y - 0.3F) > 1.0e-5F ||
      std::abs(spawn.position.z - 44.0F) > 1.0e-5F ||
      std::ranges::any_of(spawn.wheels, [](const auto& wheel) { return wheel.contact; })) {
    std::cerr << "vehicle spawn state is incorrect\n";
    return 1;
  }

  tmnf::gameplay::VehicleState visualSteeringState = spawn;
  visualSteeringState.smoothedSteering = 1.0F;
  if (!simulation.restore(visualSteeringState) ||
      std::abs(simulation.frontWheelSteeringRadians() - 0.523598776F) > 1.0e-6F) {
    std::cerr << "zero-speed visual steering did not use the authored 30-degree key\n";
    return 1;
  }
  visualSteeringState = spawn;
  visualSteeringState.smoothedSteering = -1.0F;
  visualSteeringState.speed = -100.0F / 3.6F;
  visualSteeringState.horizontalVelocityX =
      visualSteeringState.speed * std::sin(visualSteeringState.heading);
  visualSteeringState.horizontalVelocityZ =
      visualSteeringState.speed * std::cos(visualSteeringState.heading);
  visualSteeringState.gear = -1;
  visualSteeringState.engineRpm = 0.0F;
  visualSteeringState.engineTargetRpm = 0.0F;
  if (!simulation.restore(visualSteeringState) ||
      std::abs(simulation.frontWheelSteeringRadians() + 0.174532925F) > 1.0e-6F) {
    std::cerr << "reverse 100-km/h visual steering did not use the authored 10-degree key\n";
    return 1;
  }
  if (!simulation.restore(spawn)) {
    std::cerr << "vehicle spawn could not be restored after visual steering checks\n";
    return 1;
  }

  tmnf::gbx::VehicleTuning lateralTuning = tuning;
  lateralTuning.groundSlowDownCoef = 0.0F;
  tmnf::gameplay::VehicleSimulation lateralSimulation;
  if (!lateralSimulation.initialize(scene, collision.collision, lateralTuning)) {
    std::cerr << "lateral tire fixture did not initialize\n";
    return 1;
  }
  tmnf::gameplay::VehicleState lateralState = spawn;
  lateralState.position.y = 0.2F;
  lateralState.horizontalVelocityX = 10.0F;
  if (!lateralSimulation.restore(lateralState)) {
    std::cerr << "lateral tire state could not be restored\n";
    return 1;
  }
  constexpr float kStepSeconds = 1.0F / 120.0F;

  tmnf::content::TrackScene tinyTailScene = makeScene();
  tinyTailScene.blockDefinitions.front().metadata.spawnAir.value()[10] = 0.21F;
  const tmnf::content::TrackCollisionBuildResult tinyTailCollision =
      tmnf::content::buildTrackCollision(tinyTailScene);
  tmnf::gameplay::VehicleSimulation combinedStepSimulation;
  tmnf::gameplay::VehicleSimulation equalStepSimulation;
  if (!tinyTailCollision ||
      !combinedStepSimulation.initialize(tinyTailScene, tinyTailCollision.collision, tuning) ||
      !equalStepSimulation.initialize(tinyTailScene, tinyTailCollision.collision, tuning)) {
    std::cerr << "tiny-tail substep fixture did not initialize\n";
    return 1;
  }
  constexpr float kTinyTailSeconds = 0.0000006663F;
  const float combinedStepSeconds = kStepSeconds + kTinyTailSeconds;
  const tmnf::gameplay::VehicleControl launchControl{.throttle = 1.0F};
  combinedStepSimulation.update(combinedStepSeconds, launchControl);
  equalStepSimulation.update(combinedStepSeconds * 0.5F, launchControl);
  equalStepSimulation.update(combinedStepSeconds * 0.5F, launchControl);
  if (std::abs(combinedStepSimulation.state().priorLocalForcePerInertialMass.y -
               equalStepSimulation.state().priorLocalForcePerInertialMass.y) > 1.0e-5F ||
      std::abs(combinedStepSimulation.state().wheels[0].suspensionSpeed -
               equalStepSimulation.state().wheels[0].suspensionSpeed) > 1.0e-5F) {
    std::cerr << "tiny frame remainder created an unequal suspension substep\n";
    return 1;
  }

  lateralSimulation.update(kStepSeconds, {});
  const float compressionCoordinate =
      (tuning.suspensionRest - tuning.suspensionMaximum) /
      (tuning.suspensionMinimum - tuning.suspensionMaximum);
  const float compressionFactor =
      std::lerp(0.3F, 1.0F, (compressionCoordinate - 0.6F) / (0.975F - 0.6F));
  const float sideForceCap = 80.0F * compressionFactor;
  const float sideForce = std::lerp(-sideForceCap, -200.0F,
                                    tuning.maxSideFrictionBlendCoef);
  const float expectedLateralVelocity = 10.0F + 4.0F * sideForce * kStepSeconds;
  if (std::abs(lateralSimulation.state().horizontalVelocityX - expectedLateralVelocity) >
          1.0e-5F ||
      !std::ranges::all_of(lateralSimulation.state().wheels,
                           [](const auto& wheel) { return wheel.slipping; })) {
    std::cerr << "grounded lateral tire cap mismatch: vx="
              << lateralSimulation.state().horizontalVelocityX << "/"
              << expectedLateralVelocity << ", delta="
              << (lateralSimulation.state().horizontalVelocityX - expectedLateralVelocity)
              << ", wheels=";
    for (const auto& wheel : lateralSimulation.state().wheels) {
      std::cerr << wheel.contact << ':' << wheel.slipping << ' ';
    }
    std::cerr << '\n';
    return 1;
  }

  tmnf::gameplay::VehicleSimulation tiltedLateralSimulation;
  if (!tiltedLateralSimulation.initialize(scene, collision.collision, lateralTuning)) {
    std::cerr << "tilted lateral tire fixture did not initialize\n";
    return 1;
  }
  tmnf::gameplay::VehicleState tiltedLateralState = spawn;
  constexpr float kRollCos30 = 0.866025404F;
  constexpr float kRollSin30 = 0.5F;
  tiltedLateralState.position.y = 10.0F;
  tiltedLateralState.orientationWxyz = {kRollCos30, 0.0F, 0.0F, kRollSin30};
  tiltedLateralState.horizontalVelocityX = 10.0F * 0.5F;
  tiltedLateralState.verticalVelocity = 10.0F * kRollCos30;
  tiltedLateralState.horizontalVelocityZ = 0.0F;
  tiltedLateralState.speed = 0.0F;
  if (!tiltedLateralSimulation.restore(tiltedLateralState)) {
    std::cerr << "tilted lateral tire state could not be restored\n";
    return 1;
  }
  tiltedLateralSimulation.update(kStepSeconds, {});
  constexpr float kLateralAdherenceAtHalfLoad = 0.292893219F;
  const float tiltedSideForceCap =
      80.0F * kLateralAdherenceAtHalfLoad * compressionFactor;
  const float tiltedSideForce =
      std::lerp(-tiltedSideForceCap, -200.0F, tuning.maxSideFrictionBlendCoef);
  const float expectedTiltedLocalX =
      10.0F + (-10.0F * tuning.gravityCoef * kRollCos30 + 4.0F * tiltedSideForce) *
                  kStepSeconds;
  const auto& tiltedLateral = tiltedLateralSimulation.state();
  const float actualTiltedLocalX =
      0.5F * tiltedLateral.horizontalVelocityX +
      kRollCos30 * tiltedLateral.verticalVelocity;
  if (std::abs(actualTiltedLocalX - expectedTiltedLocalX) > 1.0e-5F) {
    std::cerr << "lateral slope-adherence ownership mismatch: vx="
              << actualTiltedLocalX << "/" << expectedTiltedLocalX << '\n';
    return 1;
  }

  tmnf::gameplay::VehicleSimulation inertialTorqueSimulation;
  if (!inertialTorqueSimulation.initialize(scene, collision.collision, tuning)) {
    std::cerr << "inertial torque fixture did not initialize\n";
    return 1;
  }
  tmnf::gameplay::VehicleState inertialTorqueState = spawn;
  inertialTorqueState.position.y = 10.0F;
  inertialTorqueState.priorLocalForcePerInertialMass = {2.0F, 0.0F, 0.0F};
  for (auto& wheel : inertialTorqueState.wheels) {
    wheel = {};
    wheel.suspensionLength = tuning.suspensionRest;
  }
  inertialTorqueState.wheels[0].contact = true;
  inertialTorqueState.wheels[0].contactPoint = inertialTorqueState.position;
  if (!inertialTorqueSimulation.restore(inertialTorqueState)) {
    std::cerr << "inertial torque state could not be restored\n";
    return 1;
  }
  inertialTorqueSimulation.update(kStepSeconds, {});
  const float centerOfMassY =
      (0.352499992F + 0.352499992F + 0.352503985F + 0.352503985F) * 0.25F -
      0.364F + tuning.centerOfMassDownUp;
  constexpr float kInverseInertiaZ = 1.2F;
  const float expectedInertialAngularVelocityZ =
      centerOfMassY * 10.0F * tuning.m6InertialTorqueModulationZ * kInverseInertiaZ *
      kStepSeconds;
  if (std::abs(inertialTorqueSimulation.state().angularVelocity.z -
               expectedInertialAngularVelocityZ) > 1.0e-6F) {
    std::cerr << "contact inertial torque mismatch: wz="
              << inertialTorqueSimulation.state().angularVelocity.z << "/"
              << expectedInertialAngularVelocityZ << '\n';
    return 1;
  }

  tmnf::gameplay::VehicleSimulation yawTorqueSimulation;
  if (!yawTorqueSimulation.initialize(scene, collision.collision, tuning)) {
    std::cerr << "axle yaw torque fixture did not initialize\n";
    return 1;
  }
  tmnf::gameplay::VehicleState yawTorqueState = spawn;
  yawTorqueState.position.y = 10.0F;
  yawTorqueState.horizontalVelocityX = 10.0F;
  for (auto& wheel : yawTorqueState.wheels) {
    wheel = {};
    wheel.suspensionLength = tuning.suspensionRest;
  }
  yawTorqueState.wheels[0].contact = true;
  if (!yawTorqueSimulation.restore(yawTorqueState)) {
    std::cerr << "axle yaw torque state could not be restored\n";
    return 1;
  }
  yawTorqueSimulation.update(kStepSeconds, {});
  const float halfWheelbase = (1.78208899F - -1.20550203F) * 0.5F;
  constexpr float kInverseInertiaY = 0.48F;
  constexpr float expectedYawAngularVelocity = 0.0F;
  if (std::abs(yawTorqueSimulation.state().angularVelocity.y -
               expectedYawAngularVelocity) > 1.0e-6F) {
    std::cerr << "source all-axle yaw cancellation mismatch: wy="
              << yawTorqueSimulation.state().angularVelocity.y << "/"
              << expectedYawAngularVelocity << '\n';
    return 1;
  }

  tmnf::gameplay::VehicleSimulation tiltedYawSimulation;
  if (!tiltedYawSimulation.initialize(scene, collision.collision, tuning)) {
    std::cerr << "tilted axle yaw fixture did not initialize\n";
    return 1;
  }
  tmnf::gameplay::VehicleState tiltedYawState = spawn;
  tiltedYawState.position.y = 10.0F;
  tiltedYawState.orientationWxyz = {kRollCos30, 0.0F, 0.0F, kRollSin30};
  tiltedYawState.angularVelocity = {-100.0F * kRollCos30, 50.0F, 0.0F};
  for (auto& wheel : tiltedYawState.wheels) {
    wheel = {};
    wheel.suspensionLength = tuning.suspensionRest;
  }
  tiltedYawState.wheels[0].contact = true;
  if (!tiltedYawSimulation.restore(tiltedYawState)) {
    std::cerr << "tilted axle yaw state could not be restored\n";
    return 1;
  }
  tiltedYawSimulation.update(kStepSeconds, {});
  const float rawYawForceMagnitude =
      0.5F * tuning.sideFriction1 * 100.0F * halfWheelbase;
  const float limitedYawForceMagnitude =
      std::lerp(80.0F, rawYawForceMagnitude, tuning.maxSideFrictionBlendCoef);
  const float expectedTiltedLocalYaw =
      100.0F - 4.0F * tuning.steerGroundTorque * limitedYawForceMagnitude *
                   halfWheelbase * kInverseInertiaY * kStepSeconds;
  const auto& tiltedYaw = tiltedYawSimulation.state();
  const float actualTiltedLocalYaw =
      -kRollCos30 * tiltedYaw.angularVelocity.x +
      0.5F * tiltedYaw.angularVelocity.y;
  if (std::abs(actualTiltedLocalYaw - expectedTiltedLocalYaw) > 1.0e-5F) {
    std::cerr << "yaw cap used slope adherence or skipped unsupported axles: wy="
              << actualTiltedLocalYaw << "/" << expectedTiltedLocalYaw << '\n';
    return 1;
  }

  tmnf::gameplay::VehicleSimulation lowSpeedSteerSimulation;
  if (!lowSpeedSteerSimulation.initialize(scene, collision.collision, tuning)) {
    std::cerr << "low-speed steering fixture did not initialize\n";
    return 1;
  }
  tmnf::gameplay::VehicleState lowSpeedSteerState = spawn;
  lowSpeedSteerState.position.y = 10.0F;
  lowSpeedSteerState.speed = 15.0F;
  lowSpeedSteerState.horizontalVelocityZ = 15.0F;
  lowSpeedSteerState.smoothedSteering = 1.0F;
  for (auto& wheel : lowSpeedSteerState.wheels) {
    wheel = {};
    wheel.suspensionLength = tuning.suspensionRest;
  }
  lowSpeedSteerState.wheels[0].contact = true;
  if (!lowSpeedSteerSimulation.restore(lowSpeedSteerState)) {
    std::cerr << "low-speed steering state could not be restored\n";
    return 1;
  }
  lowSpeedSteerSimulation.update(kStepSeconds, {.steering = 1.0F});
  const float lowSpeedFactor = std::sin(15.0F / tuning.steerLowSpeed * 1.57079632679F);
  const float lowSpeedDriveTorque = std::lerp(10.4F, 8.5F, 0.08F);
  const float expectedLowSpeedYaw = 2.0F * lowSpeedFactor * lowSpeedDriveTorque *
                                    halfWheelbase * kInverseInertiaY * kStepSeconds;
  if (std::abs(lowSpeedSteerSimulation.state().angularVelocity.y -
               expectedLowSpeedYaw) > 1.0e-6F) {
    std::cerr << "low-speed steering fade mismatch: wy="
              << lowSpeedSteerSimulation.state().angularVelocity.y << "/"
              << expectedLowSpeedYaw << '\n';
    return 1;
  }

  tmnf::gameplay::VehicleState forwardLatchedSteer = lowSpeedSteerState;
  forwardLatchedSteer.reverseDirectionLatched = false;
  tmnf::gameplay::VehicleState reverseLatchedSteer = lowSpeedSteerState;
  reverseLatchedSteer.reverseDirectionLatched = true;
  tmnf::gameplay::VehicleSimulation forwardLatchSimulation;
  tmnf::gameplay::VehicleSimulation reverseLatchSimulation;
  const tmnf::gameplay::VehicleControl latchHoldControl{
      .steering = 1.0F, .throttle = 0.1F, .brake = 0.1F};
  if (!forwardLatchSimulation.initialize(scene, collision.collision, tuning) ||
      !reverseLatchSimulation.initialize(scene, collision.collision, tuning) ||
      !forwardLatchSimulation.restore(forwardLatchedSteer) ||
      !reverseLatchSimulation.restore(reverseLatchedSteer)) {
    std::cerr << "reverse-direction latch fixtures did not initialize\n";
    return 1;
  }
  forwardLatchSimulation.update(kStepSeconds, latchHoldControl);
  reverseLatchSimulation.update(kStepSeconds, latchHoldControl);
  if (forwardLatchSimulation.state().angularVelocity.y <= 0.0F ||
      reverseLatchSimulation.state().angularVelocity.y >= 0.0F ||
      std::abs(forwardLatchSimulation.state().angularVelocity.y +
               reverseLatchSimulation.state().angularVelocity.y) > 1.0e-6F ||
      forwardLatchSimulation.state().reverseDirectionLatched ||
      !reverseLatchSimulation.state().reverseDirectionLatched) {
    std::cerr << "steering direction was inferred from velocity instead of +0x5C4\n";
    return 1;
  }

  tmnf::gameplay::VehicleState latchAtRest = lowSpeedSteerState;
  latchAtRest.speed = 0.0F;
  latchAtRest.horizontalVelocityZ = 0.0F;
  latchAtRest.smoothedSteering = 0.0F;
  latchAtRest.engineRpm = 0.0F;
  latchAtRest.engineTargetRpm = 0.0F;
  if (!reverseLatchSimulation.restore(latchAtRest)) {
    std::cerr << "stationary reverse-latch state could not be restored\n";
    return 1;
  }
  reverseLatchSimulation.update(kStepSeconds, {.brake = 0.2F});
  const tmnf::gameplay::VehicleState savedReverseLatch =
      reverseLatchSimulation.state();
  tmnf::gameplay::VehicleSimulation restoredReverseLatchSimulation;
  if (!savedReverseLatch.reverseDirectionLatched ||
      !restoredReverseLatchSimulation.initialize(scene, collision.collision, tuning) ||
      !restoredReverseLatchSimulation.restore(savedReverseLatch)) {
    std::cerr << "brake hysteresis or reverse-latch snapshot restore failed\n";
    return 1;
  }
  restoredReverseLatchSimulation.update(kStepSeconds, {});
  if (!restoredReverseLatchSimulation.state().reverseDirectionLatched) {
    std::cerr << "airborne step did not preserve the reverse-direction latch\n";
    return 1;
  }
  restoredReverseLatchSimulation.reset();
  if (restoredReverseLatchSimulation.state().reverseDirectionLatched) {
    std::cerr << "respawn did not clear the reverse-direction latch\n";
    return 1;
  }

  tmnf::gbx::VehicleTuning steerSlowDownTuning = tuning;
  steerSlowDownTuning.groundSlowDownCoef = 0.0F;
  tmnf::gameplay::VehicleSimulation steerSlowDownSimulation;
  if (!steerSlowDownSimulation.initialize(scene, collision.collision,
                                           steerSlowDownTuning)) {
    std::cerr << "steering slowdown fixture did not initialize\n";
    return 1;
  }
  tmnf::gameplay::VehicleState steerSlowDownState = spawn;
  steerSlowDownState.position.y = 10.0F;
  steerSlowDownState.speed = -10.0F;
  steerSlowDownState.horizontalVelocityZ = -10.0F;
  steerSlowDownState.gear = -1;
  steerSlowDownState.smoothedSteering = 1.0F;
  for (auto& wheel : steerSlowDownState.wheels) {
    wheel = {};
    wheel.suspensionLength = tuning.suspensionRest;
  }
  steerSlowDownState.wheels[2].contact = true;
  if (!steerSlowDownSimulation.restore(steerSlowDownState)) {
    std::cerr << "steering slowdown state could not be restored\n";
    return 1;
  }
  steerSlowDownSimulation.update(kStepSeconds, {.steering = 1.0F});
  const float expectedReverseSteerSlowDown =
      -10.0F + 0.5F * 0.72F * kStepSeconds / tuning.mass;
  if (std::abs(steerSlowDownSimulation.state().speed -
               expectedReverseSteerSlowDown) > 1.0e-5F) {
    std::cerr << "source steering slowdown curve mismatch: speed="
              << steerSlowDownSimulation.state().speed << "/"
              << expectedReverseSteerSlowDown << '\n';
    return 1;
  }

  tmnf::gameplay::VehicleSimulation rolloverSimulation;
  if (!rolloverSimulation.initialize(scene, collision.collision, tuning)) {
    std::cerr << "regular rollover fixture did not initialize\n";
    return 1;
  }
  tmnf::gameplay::VehicleState rolloverState = spawn;
  rolloverState.position.y = 10.0F;
  rolloverState.grounded = false;
  rolloverState.speed = 20.0F;
  rolloverState.horizontalVelocityX = 10.0F;
  rolloverState.horizontalVelocityZ = 20.0F;
  for (auto& wheel : rolloverState.wheels) {
    wheel = {};
    wheel.suspensionLength = tuning.suspensionRest;
  }
  if (!rolloverSimulation.restore(rolloverState)) {
    std::cerr << "regular rollover state could not be restored\n";
    return 1;
  }
  rolloverSimulation.update(kStepSeconds, {});
  constexpr float kExpectedRolloverTorque = -1.0F;
  const float expectedRolloverAngularVelocity =
      kExpectedRolloverTorque * kInverseInertiaZ * kStepSeconds;
  if (std::abs(rolloverSimulation.state().angularVelocity.z -
               expectedRolloverAngularVelocity) > 1.0e-6F) {
    std::cerr << "regular rollover torque mismatch: wz="
              << rolloverSimulation.state().angularVelocity.z << "/"
              << expectedRolloverAngularVelocity << '\n';
    return 1;
  }

  tmnf::gbx::VehicleTuning dragTuning = tuning;
  dragTuning.mass = 2.0F;
  dragTuning.groundSlowDownCoef = 0.0F;
  tmnf::gameplay::VehicleSimulation forceOwnershipSimulation;
  if (!forceOwnershipSimulation.initialize(scene, collision.collision, dragTuning)) {
    std::cerr << "force ownership fixture did not initialize\n";
    return 1;
  }
  tmnf::gameplay::VehicleState forceState = spawn;
  forceState.position.y = 0.2F;
  forceState.speed = 10.0F;
  forceState.horizontalVelocityZ = 10.0F;
  forceState.engineRpm = 3'700.0F;
  forceState.engineTargetRpm = 3'700.0F;
  if (!forceOwnershipSimulation.restore(forceState)) {
    std::cerr << "ground drag fixture could not be restored\n";
    return 1;
  }
  forceOwnershipSimulation.update(kStepSeconds, {});
  if (!forceOwnershipSimulation.state().grounded ||
      std::abs(forceOwnershipSimulation.state().speed - 10.0F) > 1.0e-5F) {
    std::cerr << "linear fluid friction was applied with ground contact\n";
    return 1;
  }
  forceState.position.y = 10.0F;
  forceState.orientationWxyz = {1.0F, 0.0F, 0.0F, 0.0F};
  forceState.angularVelocity = {3.0F, 0.0F, 0.0F};
  forceState.grounded = true;
  if (!forceOwnershipSimulation.restore(forceState)) {
    std::cerr << "grounded angular drag fixture could not be restored\n";
    return 1;
  }
  forceOwnershipSimulation.update(kStepSeconds, {});
  if (std::abs(forceOwnershipSimulation.state().angularVelocity.x - 3.0F) > 1.0e-6F) {
    std::cerr << "angular fluid friction was applied with tire support\n";
    return 1;
  }
  forceState.grounded = false;
  forceState.angularVelocity = {};
  forceState.verticalVelocity = 6.0F;
  if (!forceOwnershipSimulation.restore(forceState)) {
    std::cerr << "air drag fixture could not be restored\n";
    return 1;
  }
  forceOwnershipSimulation.update(kStepSeconds, {});
  const float expectedFluidScale =
      1.0F - dragTuning.linearFluidFrictionCoef / dragTuning.mass * kStepSeconds;
  const float expectedAirSpeed = 10.0F * expectedFluidScale;
  const float expectedAirVerticalVelocity =
      6.0F * expectedFluidScale - 10.0F * dragTuning.gravityCoefAir * kStepSeconds;
  if (forceOwnershipSimulation.state().grounded ||
      std::abs(forceOwnershipSimulation.state().speed - expectedAirSpeed) > 1.0e-5F ||
      std::abs(forceOwnershipSimulation.state().verticalVelocity -
               expectedAirVerticalVelocity) > 1.0e-5F) {
    std::cerr << "linear fluid force was not applied to all axes through mass\n";
    return 1;
  }
  tmnf::gameplay::VehicleState angularDrag = spawn;
  angularDrag.position.y = 10.0F;
  angularDrag.orientationWxyz = {1.0F, 0.0F, 0.0F, 0.0F};
  angularDrag.angularVelocity = {3.0F, 0.0F, 0.0F};
  angularDrag.grounded = false;
  if (!forceOwnershipSimulation.restore(angularDrag)) {
    std::cerr << "air angular drag fixture could not be restored\n";
    return 1;
  }
  forceOwnershipSimulation.update(kStepSeconds, {});
  const float expectedAirAngularVelocity =
      3.0F - 3.0F * (dragTuning.angularFluidFrictionCoef1 +
                     3.0F * dragTuning.angularFluidFrictionCoef2) * 0.48F * kStepSeconds;
  if (std::abs(forceOwnershipSimulation.state().angularVelocity.x -
               expectedAirAngularVelocity) > 1.0e-5F) {
    std::cerr << "air angular fluid friction mismatch: omegaX="
              << forceOwnershipSimulation.state().angularVelocity.x << "/"
              << expectedAirAngularVelocity << '\n';
    return 1;
  }

  tmnf::gameplay::VehicleState heldAirYaw = angularDrag;
  heldAirYaw.angularVelocity = {0.0F, 1.0F, 0.0F};
  heldAirYaw.cachedLocalAngularVelocity = {0.0F, -1.0F, 0.0F};
  heldAirYaw.smoothedSteering = 1.0F;
  heldAirYaw.elapsedSeconds = 1.0F;
  heldAirYaw.lastWheelContactSeconds = 0.0F;
  if (!forceOwnershipSimulation.restore(heldAirYaw)) {
    std::cerr << "recent-contact yaw-hold state could not be restored\n";
    return 1;
  }
  forceOwnershipSimulation.update(kStepSeconds, {.steering = 1.0F});
  const float expectedHeldAirYaw =
      -1.0F - (dragTuning.angularFluidFrictionCoef1 +
               dragTuning.angularFluidFrictionCoef2) *
                  0.48F * kStepSeconds;
  if (std::abs(forceOwnershipSimulation.state().angularVelocity.y -
               expectedHeldAirYaw) > 1.0e-5F ||
      forceOwnershipSimulation.state().cachedLocalAngularVelocity.y != -1.0F) {
    std::cerr << "opposite air steering did not hold cached yaw below 3.14\n";
    return 1;
  }

  tmnf::gameplay::VehicleState boostedAirYaw = heldAirYaw;
  boostedAirYaw.angularVelocity.y = 2.0F;
  boostedAirYaw.cachedLocalAngularVelocity.y = 1.0F;
  if (!forceOwnershipSimulation.restore(boostedAirYaw)) {
    std::cerr << "recent-contact yaw-boost state could not be restored\n";
    return 1;
  }
  forceOwnershipSimulation.update(kStepSeconds, {.steering = 1.0F});
  const float boostedDragSpeed = 6.0F;
  const float boostedDragTorque =
      dragTuning.angularFluidFrictionCoef1 * boostedDragSpeed +
      dragTuning.angularFluidFrictionCoef2 * boostedDragSpeed * boostedDragSpeed;
  const float expectedBoostedAirYaw =
      2.0F - boostedDragTorque * 0.48F * kStepSeconds;
  if (std::abs(forceOwnershipSimulation.state().angularVelocity.y -
               expectedBoostedAirYaw) > 1.0e-5F ||
      forceOwnershipSimulation.state().cachedLocalAngularVelocity.y != 2.0F) {
    std::cerr << "same-direction air steering did not use nonlinear 3x drag\n";
    return 1;
  }

  tmnf::gameplay::VehicleState collisionPulseYaw = boostedAirYaw;
  collisionPulseYaw.bodyCollisionPulse = true;
  if (!forceOwnershipSimulation.restore(collisionPulseYaw)) {
    std::cerr << "collision-pulse yaw state could not be restored\n";
    return 1;
  }
  forceOwnershipSimulation.update(kStepSeconds, {.steering = 1.0F});
  const float ordinaryYawTorque =
      dragTuning.angularFluidFrictionCoef1 * 2.0F +
      dragTuning.angularFluidFrictionCoef2 * 4.0F;
  const float expectedCollisionPulseYaw =
      2.0F - ordinaryYawTorque * 0.48F * kStepSeconds;
  if (std::abs(forceOwnershipSimulation.state().angularVelocity.y -
               expectedCollisionPulseYaw) > 1.0e-5F ||
      forceOwnershipSimulation.state().cachedLocalAngularVelocity.y != 2.0F ||
      forceOwnershipSimulation.state().bodyCollisionPulse) {
    std::cerr << "collision pulse did not refresh cache and bypass air-control boost\n";
    return 1;
  }

  tmnf::gameplay::VehicleState brakedAirPitch = heldAirYaw;
  brakedAirPitch.angularVelocity = {3.0F, 0.0F, 0.0F};
  brakedAirPitch.cachedLocalAngularVelocity = {1.0F, 0.0F, 0.0F};
  brakedAirPitch.smoothedSteering = 0.0F;
  if (!forceOwnershipSimulation.restore(brakedAirPitch)) {
    std::cerr << "air-brake pitch state could not be restored\n";
    return 1;
  }
  forceOwnershipSimulation.update(kStepSeconds, {.brake = 0.2F});
  const float expectedBrakedAirPitch =
      -(dragTuning.angularFluidFrictionCoef1 * 3.0F +
        dragTuning.angularFluidFrictionCoef2 * 9.0F) *
      0.48F * kStepSeconds;
  if (std::abs(forceOwnershipSimulation.state().angularVelocity.x -
               expectedBrakedAirPitch) > 1.0e-5F ||
      forceOwnershipSimulation.state().cachedLocalAngularVelocity.x != 0.0F) {
    std::cerr << "recent-contact air brake did not clamp positive local pitch\n";
    return 1;
  }

  tmnf::gameplay::VehicleState seededAngularHistory = angularDrag;
  seededAngularHistory.angularVelocity = {1.0F, 2.0F, 3.0F};
  seededAngularHistory.grounded = true;
  seededAngularHistory.elapsedSeconds = 2.0F;
  seededAngularHistory.bodyCollisionPulse = true;
  if (!forceOwnershipSimulation.restore(seededAngularHistory)) {
    std::cerr << "ground-contact angular-history state could not be restored\n";
    return 1;
  }
  forceOwnershipSimulation.update(kStepSeconds, {});
  if (forceOwnershipSimulation.state().lastWheelContactSeconds != 2.0F ||
      forceOwnershipSimulation.state().cachedLocalAngularVelocity.x != 1.0F ||
      forceOwnershipSimulation.state().cachedLocalAngularVelocity.y != 2.0F ||
      forceOwnershipSimulation.state().cachedLocalAngularVelocity.z != 3.0F ||
      forceOwnershipSimulation.state().bodyCollisionPulse) {
    std::cerr << "qualified wheel contact did not win angular-history branch order\n";
    return 1;
  }

  tmnf::gbx::VehicleTuning gravityTuning = dragTuning;
  gravityTuning.linearFluidFrictionCoef = 0.0F;
  if (!forceOwnershipSimulation.initialize(scene, collision.collision, gravityTuning)) {
    std::cerr << "gravity ownership fixture did not initialize\n";
    return 1;
  }
  forceState = spawn;
  forceState.position.z = 161.3F;
  forceState.speed = 10.0F;
  forceState.horizontalVelocityZ = 10.0F;
  forceState.engineRpm = 3'700.0F;
  forceState.engineTargetRpm = 3'700.0F;
  if (!forceOwnershipSimulation.restore(forceState)) {
    std::cerr << "contact gravity fixture could not be restored\n";
    return 1;
  }
  forceOwnershipSimulation.update(kStepSeconds, {});
  const float expectedContactGravity = -10.0F * gravityTuning.gravityCoef * kStepSeconds;
  if (forceOwnershipSimulation.state().grounded ||
      std::abs(forceOwnershipSimulation.state().verticalVelocity - expectedContactGravity) >
          1.0e-5F) {
    std::cerr << "contact gravity ownership was not retained on takeoff\n";
    return 1;
  }
  forceState = spawn;
  forceState.position.y = 10.0F;
  forceState.grounded = false;
  if (!forceOwnershipSimulation.restore(forceState)) {
    std::cerr << "air gravity fixture could not be restored\n";
    return 1;
  }
  forceOwnershipSimulation.update(kStepSeconds, {});
  const float expectedAirGravity = -10.0F * gravityTuning.gravityCoefAir * kStepSeconds;
  if (forceOwnershipSimulation.state().grounded ||
      std::abs(forceOwnershipSimulation.state().verticalVelocity - expectedAirGravity) >
          1.0e-5F) {
    std::cerr << "air gravity coefficient was not applied\n";
    return 1;
  }

  constexpr float kSin30 = 0.5F;
  constexpr float kCos30 = 0.866025404F;
  constexpr std::array<float, 4> kUphillOrientation{0.965925826F, -0.258819045F, 0.0F,
                                                    0.0F};
  const auto verifyInclineControl = [&](const float initialSpeed,
                                        const tmnf::gameplay::VehicleControl control,
                                        const float expectedDriveDelta) {
    tmnf::gameplay::VehicleState incline = spawn;
    incline.position.y = 10.0F;
    incline.orientationWxyz = kUphillOrientation;
    incline.horizontalVelocityX = 0.0F;
    incline.verticalVelocity = initialSpeed * kSin30;
    incline.horizontalVelocityZ = initialSpeed * kCos30;
    incline.speed = initialSpeed;
    incline.engineRpm = std::copysign(std::abs(initialSpeed) * 370.0F, initialSpeed);
    incline.engineTargetRpm = std::abs(incline.engineRpm);
    if (!forceOwnershipSimulation.restore(incline)) {
      return false;
    }
    const tmnf::gameplay::VehicleState before = forceOwnershipSimulation.state();
    forceOwnershipSimulation.update(kStepSeconds, control);
    const tmnf::gameplay::VehicleState after = forceOwnershipSimulation.state();
    const float driveDeltaX = after.horizontalVelocityX - before.horizontalVelocityX;
    const float driveDeltaY = after.verticalVelocity - before.verticalVelocity -
                              expectedContactGravity;
    const float driveDeltaZ = after.horizontalVelocityZ - before.horizontalVelocityZ;
    const float alongBodyUp = driveDeltaY * kCos30 - driveDeltaZ * kSin30;
    const float expectedLocalSpeed =
        initialSpeed + expectedDriveDelta + expectedContactGravity * kSin30;
    if (std::abs(driveDeltaX) > 1.0e-5F ||
        std::abs(driveDeltaY - expectedDriveDelta * kSin30) > 1.0e-5F ||
        std::abs(driveDeltaZ - expectedDriveDelta * kCos30) > 1.0e-5F ||
        std::abs(alongBodyUp) > 1.0e-5F ||
        std::abs(after.speed - expectedLocalSpeed) > 1.0e-5F) {
      std::cerr << "incline mismatch: initial=" << initialSpeed
                << ", expectedDrive=" << expectedDriveDelta << ", delta=(" << driveDeltaX
                << ',' << driveDeltaY << ',' << driveDeltaZ << "), bodyUp=" << alongBodyUp
                << ", speed=" << after.speed << ", expectedSpeed=" << expectedLocalSpeed
                << '\n';
      return false;
    }
    tmnf::gameplay::VehicleSimulation restored;
    return restored.initialize(scene, collision.collision, gravityTuning) &&
           restored.restore(after);
  };
  const float inclineAcceleration =
      (16.0F + (11.0F - 16.0F) * (36.0F / 101.0F)) /
      gravityTuning.mass * kStepSeconds;
  if (!verifyInclineControl(10.0F, {.throttle = 1.0F}, inclineAcceleration) ||
      !verifyInclineControl(10.0F, {.brake = 1.0F},
                            -gravityTuning.brakeMaxDynamic * kStepSeconds) ||
      !verifyInclineControl(0.0F, {.brake = 1.0F},
                            -30.0F / gravityTuning.mass * kStepSeconds)) {
    std::cerr << "incline drive/brake force did not remain on body-local Z\n";
    return 1;
  }
  tmnf::gameplay::VehicleState mismatchedLocalSpeed = spawn;
  mismatchedLocalSpeed.orientationWxyz = kUphillOrientation;
  mismatchedLocalSpeed.verticalVelocity = 0.0F;
  mismatchedLocalSpeed.horizontalVelocityZ = 10.0F;
  mismatchedLocalSpeed.speed = 10.0F;
  if (forceOwnershipSimulation.restore(mismatchedLocalSpeed)) {
    std::cerr << "restore accepted speed that did not match body-local Z velocity\n";
    return 1;
  }

  const tmnf::content::TrackScene wheelSupportScene = makeWheelSupportScene();
  const tmnf::content::TrackCollisionBuildResult wheelSupportCollision =
      tmnf::content::buildTrackCollision(wheelSupportScene);
  tmnf::gameplay::VehicleSimulation wheelSupportSimulation;
  if (!wheelSupportCollision ||
      !wheelSupportSimulation.initialize(wheelSupportScene, wheelSupportCollision.collision,
                                         tuning)) {
    std::cerr << "wheel support fixture did not initialize\n";
    return 1;
  }
  tmnf::gameplay::VehicleState wheelEdge = wheelSupportSimulation.state();
  wheelEdge.position = {80.0F, 0.2F, 80.0F};
  wheelEdge.heading = 0.0F;
  if (!wheelSupportSimulation.restore(wheelEdge)) {
    std::cerr << "wheel support state could not be restored\n";
    return 1;
  }
  wheelSupportSimulation.update(kStepSeconds, {});
  if (!wheelSupportSimulation.state().grounded) {
    std::cerr << "single physical wheel support did not retain contact\n";
    return 1;
  }
  const tmnf::gameplay::VehicleState firstWheelContact = wheelSupportSimulation.state();
  const float firstCorrection = firstWheelContact.wheels[0].pendingCorrection;
  if (std::abs(firstWheelContact.position.y - 0.2F) > 1.0e-6F ||
      std::abs(firstWheelContact.wheels[0].suspensionLength - tuning.suspensionRest) > 1.0e-6F ||
      firstCorrection <= 0.0F || !firstWheelContact.wheels[0].contact ||
      firstWheelContact.wheels[1].contact || firstWheelContact.wheels[2].contact ||
      firstWheelContact.wheels[3].contact) {
    std::cerr << "wheel contact did not preserve one-substep BC latency\n";
    return 1;
  }

  tmnf::gameplay::VehicleState impulseState = wheelEdge;
  impulseState.grounded = false;
  impulseState.verticalVelocity = -10.0F;
  for (auto& wheel : impulseState.wheels) {
    wheel = {};
    wheel.suspensionLength = tuning.suspensionRest;
  }
  tmnf::gameplay::VehicleSimulation partialComplianceSimulation;
  if (!partialComplianceSimulation.initialize(wheelSupportScene,
                                               wheelSupportCollision.collision, tuning) ||
      !partialComplianceSimulation.restore(impulseState)) {
    std::cerr << "partial-compliance impulse fixture did not initialize\n";
    return 1;
  }
  partialComplianceSimulation.update(kStepSeconds, {});
  const auto& partialCompliance = partialComplianceSimulation.state();
  const float expectedPartialVerticalVelocity =
      impulseState.verticalVelocity -
      10.0F * tuning.gravityCoefAir * kStepSeconds;
  if (std::abs(partialCompliance.verticalVelocity - expectedPartialVerticalVelocity) >
          0.01F ||
      std::hypot(partialCompliance.angularVelocity.x,
                 partialCompliance.angularVelocity.z) > 1.0e-6F ||
      partialCompliance.wheels[0].pendingCorrection <= 0.0F) {
    std::cerr << "partial suspension compliance did not own its inward velocity: vy="
              << partialCompliance.verticalVelocity << "/"
              << expectedPartialVerticalVelocity << ", omega=("
              << partialCompliance.angularVelocity.x << ','
              << partialCompliance.angularVelocity.y << ','
              << partialCompliance.angularVelocity.z << "), correction="
              << partialCompliance.wheels[0].pendingCorrection << '\n';
    return 1;
  }
  impulseState.position.y -= tuning.suspensionRest - tuning.suspensionMinimum;
  for (auto& wheel : impulseState.wheels) {
    wheel.suspensionLength = tuning.suspensionMinimum;
  }
  tmnf::gbx::VehicleTuning fullImpulseTuning = tuning;
  fullImpulseTuning.angularSpeedYImpulseScale = 1.0F;
  tmnf::gameplay::VehicleSimulation scaledImpulseSimulation;
  tmnf::gameplay::VehicleSimulation fullImpulseSimulation;
  if (!scaledImpulseSimulation.initialize(wheelSupportScene, wheelSupportCollision.collision,
                                           tuning) ||
      !fullImpulseSimulation.initialize(wheelSupportScene, wheelSupportCollision.collision,
                                         fullImpulseTuning) ||
      !scaledImpulseSimulation.restore(impulseState) ||
      !fullImpulseSimulation.restore(impulseState)) {
    std::cerr << "wheel normal-impulse scale fixture did not initialize\n";
    return 1;
  }
  scaledImpulseSimulation.update(kStepSeconds, {});
  fullImpulseSimulation.update(kStepSeconds, {});
  const auto& scaledImpulse = scaledImpulseSimulation.state();
  const auto& fullImpulse = fullImpulseSimulation.state();
  const float fullAngularSpeedSquared =
      fullImpulse.angularVelocity.x * fullImpulse.angularVelocity.x +
      fullImpulse.angularVelocity.y * fullImpulse.angularVelocity.y +
      fullImpulse.angularVelocity.z * fullImpulse.angularVelocity.z;
  if (fullAngularSpeedSquared <= 1.0e-6F ||
      fullImpulse.verticalVelocity <= partialCompliance.verticalVelocity + 0.1F ||
      std::abs(scaledImpulse.angularVelocity.x -
               fullImpulse.angularVelocity.x * tuning.angularSpeedYImpulseScale) > 1.0e-5F ||
      std::abs(scaledImpulse.angularVelocity.y -
               fullImpulse.angularVelocity.y * tuning.angularSpeedYImpulseScale) > 1.0e-5F ||
      std::abs(scaledImpulse.angularVelocity.z -
               fullImpulse.angularVelocity.z * tuning.angularSpeedYImpulseScale) > 1.0e-5F ||
      std::abs(scaledImpulse.horizontalVelocityX - fullImpulse.horizontalVelocityX) > 1.0e-6F ||
      std::abs(scaledImpulse.verticalVelocity - fullImpulse.verticalVelocity) > 1.0e-6F ||
      std::abs(scaledImpulse.horizontalVelocityZ - fullImpulse.horizontalVelocityZ) > 1.0e-6F) {
    std::cerr << "wheel normal angular-impulse scale mismatch\n";
    return 1;
  }
  tmnf::content::TrackScene metalWheelSupportScene = makeWheelSupportScene();
  metalWheelSupportScene.models[0].solid.surfaceMaterials = {4};
  const auto metalWheelSupportCollision =
      tmnf::content::buildTrackCollision(metalWheelSupportScene);
  tmnf::gbx::VehicleTuning metalImpulseTuning = tuning;
  metalImpulseTuning.wheelRestCoefMetal = -0.2F;
  tmnf::gameplay::VehicleSimulation metalImpulseSimulation;
  if (!metalWheelSupportCollision ||
      !metalImpulseSimulation.initialize(metalWheelSupportScene,
                                         metalWheelSupportCollision.collision,
                                         metalImpulseTuning) ||
      !metalImpulseSimulation.restore(impulseState)) {
    std::cerr << "metal wheel impulse fixture did not initialize\n";
    return 1;
  }
  metalImpulseSimulation.update(kStepSeconds, {});
  if (metalImpulseSimulation.state().verticalVelocity <=
      scaledImpulse.verticalVelocity + 0.1F) {
    std::cerr << "surface material 4 did not select the metal wheel response\n";
    return 1;
  }
  tmnf::gameplay::VehicleSimulation restoredWheelSupportSimulation;
  if (!restoredWheelSupportSimulation.initialize(wheelSupportScene, wheelSupportCollision.collision,
                                                  tuning) ||
      !restoredWheelSupportSimulation.restore(firstWheelContact)) {
    std::cerr << "pending wheel contact could not be restored\n";
    return 1;
  }
  wheelSupportSimulation.update(kStepSeconds, {});
  restoredWheelSupportSimulation.update(kStepSeconds, {});
  const tmnf::gameplay::VehicleState secondWheelContact = wheelSupportSimulation.state();
  const float expectedWheelLength =
      tuning.suspensionRest - firstCorrection +
      tuning.absorbTension * kStepSeconds * firstCorrection;
  const float expectedWheelSpeed =
      (expectedWheelLength - tuning.suspensionRest) / kStepSeconds;
  if (std::abs(secondWheelContact.wheels[0].suspensionLength - expectedWheelLength) > 1.0e-5F ||
      std::abs(secondWheelContact.wheels[0].suspensionSpeed - expectedWheelSpeed) > 1.0e-4F ||
      std::hypot(secondWheelContact.angularVelocity.x,
                 secondWheelContact.angularVelocity.z) <= 1.0e-7F ||
      restoredWheelSupportSimulation.state().position.y != secondWheelContact.position.y ||
      restoredWheelSupportSimulation.state().wheels[0].suspensionLength !=
          secondWheelContact.wheels[0].suspensionLength ||
      restoredWheelSupportSimulation.state().angularVelocity.x !=
          secondWheelContact.angularVelocity.x ||
      restoredWheelSupportSimulation.state().angularVelocity.z !=
          secondWheelContact.angularVelocity.z) {
    std::cerr << "wheel BC, force-at-point, or restore continuation is incorrect\n";
    return 1;
  }
  wheelEdge = spawn;
  wheelEdge.position = {82.0F, 0.2F, 80.0F};
  if (!wheelSupportSimulation.restore(wheelEdge)) {
    std::cerr << "wheel clear state could not be restored\n";
    return 1;
  }
  wheelSupportSimulation.update(kStepSeconds, {});
  if (wheelSupportSimulation.state().grounded ||
      std::abs(wheelSupportSimulation.state().verticalVelocity -
               expectedContactGravity) > 1.0e-5F) {
    std::cerr << "contact persisted after all four physical wheels cleared: grounded="
              << wheelSupportSimulation.state().grounded << ", vy="
              << wheelSupportSimulation.state().verticalVelocity << ", contacts=";
    for (const auto& wheel : wheelSupportSimulation.state().wheels) {
      std::cerr << wheel.contact << ':' << wheel.pendingCorrection << ' ';
    }
    std::cerr << '\n';
    return 1;
  }
  tmnf::gameplay::VehicleState centerOnly = wheelSupportSimulation.state();
  centerOnly.position = {100.0F, 0.3F, 100.0F};
  centerOnly.verticalVelocity = -1.0F;
  if (!wheelSupportSimulation.restore(centerOnly)) {
    std::cerr << "center-only support state could not be restored\n";
    return 1;
  }
  wheelSupportSimulation.update(kStepSeconds, {});
  if (wheelSupportSimulation.state().grounded) {
    std::cerr << "center-only ground falsely replaced physical wheel contact\n";
    return 1;
  }

  simulation.update(1.0F / 120.0F, {.throttle = 1.0F});
  if (std::abs(simulation.state().speed - 0.1333F) > 0.001F ||
      simulation.state().gear != 1 ||
      std::abs(simulation.state().engineRpm - 6'000.0F / 120.0F) > 0.01F) {
    std::cerr << "source forward force curve was not applied\n";
    return 1;
  }
  simulation.reset();
  simulation.update(1.0F / 120.0F, {.brake = 1.0F});
  if (std::abs(simulation.state().speed + 0.24994F) > 0.001F ||
      simulation.state().gear != -1 ||
      std::abs(simulation.state().gearShiftSeconds - 0.025F) > 0.00001F ||
      std::abs(simulation.state().engineRpm) > 0.01F) {
    std::cerr << "source reverse force curve was not applied: speed=" << simulation.state().speed
              << ", gear=" << simulation.state().gear
              << ", shift=" << simulation.state().gearShiftSeconds
              << ", rpm=" << simulation.state().engineRpm << '\n';
    return 1;
  }
  simulation.update(1.0F / 120.0F, {.brake = 1.0F});
  if (simulation.state().gear != -1 ||
      std::abs(simulation.state().engineRpm + 6'000.0F / 120.0F) > 0.01F) {
    std::cerr << "source reverse shift free-rev rate was not applied\n";
    return 1;
  }

  tmnf::gameplay::VehicleState preShift = spawn;
  preShift.speed = 28.24F;
  preShift.horizontalVelocityZ = 28.24F;
  preShift.gear = 1;
  preShift.engineRpm = preShift.speed * 370.0F;
  preShift.engineTargetRpm = preShift.engineRpm;
  tmnf::gameplay::VehicleSimulation shiftSimulation;
  if (!shiftSimulation.initialize(scene, collision.collision, tuning) ||
      !shiftSimulation.restore(preShift)) {
    std::cerr << "drivetrain shift fixture did not initialize\n";
    return 1;
  }
  shiftSimulation.update(1.0F / 120.0F, {.throttle = 1.0F});
  const tmnf::gameplay::VehicleState shifted = shiftSimulation.state();
  if (shifted.gear != 2 || shifted.engineRpm <= preShift.engineRpm ||
      std::abs(shifted.engineRpm - (preShift.engineRpm + 6'000.0F / 120.0F)) > 0.01F ||
      std::abs(shifted.gearShiftSeconds - 0.025F) > 0.00001F) {
    std::cerr << "source drivetrain thresholds did not shift into second gear\n";
    return 1;
  }
  tmnf::gameplay::VehicleSimulation restoredShiftSimulation;
  if (!restoredShiftSimulation.initialize(scene, collision.collision, tuning) ||
      !restoredShiftSimulation.restore(shifted)) {
    std::cerr << "valid shifted drivetrain state was not restored\n";
    return 1;
  }
  shiftSimulation.update(1.0F / 120.0F, {.throttle = 0.5F});
  restoredShiftSimulation.update(1.0F / 120.0F, {.throttle = 0.5F});
  if (shiftSimulation.state().gear != restoredShiftSimulation.state().gear ||
      shiftSimulation.state().engineRpm != restoredShiftSimulation.state().engineRpm ||
      shiftSimulation.state().engineTargetRpm !=
          restoredShiftSimulation.state().engineTargetRpm ||
      shiftSimulation.state().gearShiftSeconds !=
          restoredShiftSimulation.state().gearShiftSeconds) {
    std::cerr << "restored drivetrain did not continue deterministically\n";
    return 1;
  }
  tmnf::gameplay::VehicleState invalidDrivetrain = shifted;
  invalidDrivetrain.gear = 0;
  if (restoredShiftSimulation.restore(invalidDrivetrain)) {
    std::cerr << "invalid moving drivetrain gear was accepted\n";
    return 1;
  }
  invalidDrivetrain = shifted;
  invalidDrivetrain.engineRpm = std::numeric_limits<float>::quiet_NaN();
  if (restoredShiftSimulation.restore(invalidDrivetrain)) {
    std::cerr << "invalid drivetrain RPM was accepted\n";
    return 1;
  }
  invalidDrivetrain = shifted;
  invalidDrivetrain.gearShiftSeconds = 0.026F;
  if (restoredShiftSimulation.restore(invalidDrivetrain)) {
    std::cerr << "invalid drivetrain shift timer was accepted\n";
    return 1;
  }

  simulation.reset();
  tmnf::gameplay::VehicleState rollingBackward = spawn;
  rollingBackward.speed = -10.0F;
  rollingBackward.horizontalVelocityZ = -10.0F;
  rollingBackward.gear = -1;
  rollingBackward.engineRpm = -4'000.0F;
  if (!simulation.restore(rollingBackward)) {
    std::cerr << "backward tuning fixture could not be restored\n";
    return 1;
  }
  if (std::abs(simulation.state().horizontalVelocityZ + 10.0F) > 1.0e-5F) {
    std::cerr << "backward state did not retain horizontal velocity\n";
    return 1;
  }
  simulation.update(1.0F / 120.0F, {.throttle = 1.0F});
  if (std::abs(simulation.state().speed + 9.90111F) > 0.001F) {
    std::cerr << "source forward force curve negative-speed branch was not applied\n";
    return 1;
  }
  tmnf::gameplay::VehicleState steeringFixture = spawn;
  steeringFixture.position.y = 0.2F;
  steeringFixture.speed = 10.0F;
  steeringFixture.horizontalVelocityZ = 10.0F;
  steeringFixture.gear = 1;
  steeringFixture.engineRpm = 3'700.0F;
  if (!simulation.restore(steeringFixture)) {
    std::cerr << "steering tuning fixture could not be restored\n";
    return 1;
  }
  simulation.update(0.1F, {.steering = 1.0F});
  if (std::abs(simulation.state().heading - 0.021549F) > 0.001F ||
      std::abs(simulation.state().speed - 9.82758F) > 0.001F ||
      std::abs(simulation.state().smoothedSteering - 1.0F) > 1.0e-6F) {
    std::cerr << "source smoothed tire steering/rigid-body ownership was not applied: heading="
              << simulation.state().heading << ", speed=" << simulation.state().speed
              << ", steering=" << simulation.state().smoothedSteering
              << ", grounded=" << simulation.state().grounded << '\n';
    return 1;
  }
  simulation.reset();
  for (int frame = 0; frame < 60; ++frame) {
    simulation.update(1.0F / 60.0F, {.throttle = 1.0F});
  }
  const tmnf::gameplay::VehicleState accelerated = simulation.state();
  if (!accelerated.grounded || accelerated.speed < 12.0F ||
      std::abs(accelerated.position.z - spawn.position.z) <= 5.5F) {
    std::cerr << "vehicle did not accelerate over the collision surface: grounded="
              << accelerated.grounded << ", speed=" << accelerated.speed << ", position=("
              << accelerated.position.x << ',' << accelerated.position.y << ','
              << accelerated.position.z << "), vy=" << accelerated.verticalVelocity
              << ", omega=(" << accelerated.angularVelocity.x << ','
              << accelerated.angularVelocity.y << ',' << accelerated.angularVelocity.z
              << "), wheels=";
    for (const auto& wheel : accelerated.wheels) {
      std::cerr << wheel.suspensionLength << ':' << wheel.suspensionSpeed << ':'
                << wheel.pendingCorrection << ':' << wheel.contact << ' ';
    }
    std::cerr << '\n';
    return 1;
  }
  for (int frame = 0; frame < 30; ++frame) {
    simulation.update(1.0F / 60.0F, {.steering = 1.0F, .throttle = 0.5F});
  }
  if (std::abs(simulation.state().heading - accelerated.heading) <= 0.2F ||
      std::abs(simulation.state().position.x - spawn.position.x) <= 0.5F) {
    std::cerr << "vehicle steering did not change its heading\n";
    return 1;
  }
  const float expectedResetZ = simulation.state().checkpointCount == 0U ? spawn.position.z : 68.0F;
  simulation.reset();
  if (std::abs(simulation.state().position.x - spawn.position.x) > 1.0e-5F ||
      std::abs(simulation.state().position.z - expectedResetZ) > 1.0e-5F ||
      simulation.state().speed != 0.0F) {
    std::cerr << "vehicle respawn did not restore the eligible waypoint state\n";
    return 1;
  }
  if (!simulation.restore(accelerated) || simulation.state().speed != accelerated.speed) {
    std::cerr << "valid saved vehicle state was not restored\n";
    return 1;
  }
  tmnf::gameplay::VehicleState invalid = accelerated;
  invalid.position.x = std::numeric_limits<float>::quiet_NaN();
  if (simulation.restore(invalid)) {
    std::cerr << "invalid saved vehicle state was accepted\n";
    return 1;
  }
  invalid = accelerated;
  invalid.horizontalVelocityX = std::numeric_limits<float>::quiet_NaN();
  if (simulation.restore(invalid)) {
    std::cerr << "invalid saved horizontal velocity was accepted\n";
    return 1;
  }
  invalid = accelerated;
  invalid.completedLaps = 1;
  if (simulation.restore(invalid)) {
    std::cerr << "lap progress was accepted for a point-to-point race\n";
    return 1;
  }
  tmnf::gameplay::VehicleState pitchedAndRolled = spawn;
  pitchedAndRolled.orientationWxyz = {0.9876721F, 0.0864101F, -0.0113761F, 0.1300295F};
  if (!simulation.restore(pitchedAndRolled) ||
      simulation.state().orientationWxyz != pitchedAndRolled.orientationWxyz) {
    std::cerr << "restore did not preserve the authoritative quaternion\n";
    return 1;
  }
  pitchedAndRolled.orientationWxyz[0] = std::numeric_limits<float>::quiet_NaN();
  if (simulation.restore(pitchedAndRolled)) {
    std::cerr << "invalid saved quaternion was accepted\n";
    return 1;
  }

  tmnf::gameplay::VehicleSimulation raceSimulation;
  if (!raceSimulation.initialize(scene, collision.collision, tuning) ||
      raceSimulation.checkpointTotal() != 1U) {
    std::cerr << "race progression fixture did not initialize\n";
    return 1;
  }
  for (int frame = 0; frame < 120; ++frame) {
    raceSimulation.update(1.0F / 60.0F, {.throttle = 1.0F});
  }
  if (raceSimulation.state().checkpointCount != 1U || raceSimulation.state().finished) {
    std::cerr << "vehicle did not collect the ordered checkpoint\n";
    return 1;
  }
  const float checkpointRaceTime = raceSimulation.state().raceTimeSeconds;
  raceSimulation.reset();
  if (raceSimulation.state().checkpointCount != 1U ||
      std::abs(raceSimulation.state().position.z - 68.0F) > 1.0e-5F ||
      std::abs(raceSimulation.state().position.y - 0.3F) > 1.0e-5F ||
      raceSimulation.state().raceTimeSeconds != checkpointRaceTime) {
    std::cerr << "checkpoint respawn did not preserve road position and race progress\n";
    return 1;
  }
  for (int frame = 0; frame < 150; ++frame) {
    raceSimulation.update(1.0F / 60.0F, {.throttle = 1.0F});
  }
  if (!raceSimulation.state().finished || raceSimulation.state().raceTimeSeconds <= 0.0F) {
    std::cerr << "vehicle did not finish after collecting the checkpoint\n";
    return 1;
  }

  tmnf::content::TrackScene noRespawnScene = makeScene();
  noRespawnScene.blockDefinitions[1].metadata.noRespawn = true;
  const tmnf::content::TrackCollisionBuildResult noRespawnCollision =
      tmnf::content::buildTrackCollision(noRespawnScene);
  tmnf::gameplay::VehicleSimulation noRespawnSimulation;
  if (!noRespawnCollision ||
      !noRespawnSimulation.initialize(noRespawnScene, noRespawnCollision.collision, tuning)) {
    std::cerr << "NoRespawn fixture did not initialize\n";
    return 1;
  }
  for (int frame = 0; frame < 120; ++frame) {
    noRespawnSimulation.update(1.0F / 60.0F, {.throttle = 1.0F});
  }
  noRespawnSimulation.reset();
  if (noRespawnSimulation.state().checkpointCount != 1U ||
      std::abs(noRespawnSimulation.state().position.z - spawn.position.z) > 1.0e-5F) {
    std::cerr << "NoRespawn checkpoint replaced the prior respawn location\n";
    return 1;
  }

  tmnf::content::TrackScene lapScene = makeScene();
  lapScene.challenge.isLapRace = true;
  lapScene.challenge.nbLaps = 2;
  lapScene.blockDefinitions.back().metadata.waypointType =
      tmnf::gbx::BlockWaypointType::StartFinish;
  const tmnf::content::TrackCollisionBuildResult lapCollision =
      tmnf::content::buildTrackCollision(lapScene);
  tmnf::gameplay::VehicleSimulation lapSimulation;
  if (!lapCollision || !lapSimulation.initialize(lapScene, lapCollision.collision, tuning) ||
      lapSimulation.lapTotal() != 2U) {
    std::cerr << "lap race fixture did not initialize\n";
    return 1;
  }
  for (int frame = 0; frame < 210; ++frame) {
    lapSimulation.update(1.0F / 60.0F, {.throttle = 1.0F});
  }
  if (lapSimulation.state().finished || lapSimulation.state().completedLaps != 1U ||
      lapSimulation.state().checkpointCount != 0U) {
    std::cerr << "lap race did not reset ordered checkpoint progress\n";
    return 1;
  }
  tmnf::gameplay::VehicleState secondLap = lapSimulation.state();
  secondLap.position = spawn.position;
  secondLap.groundNormal = spawn.groundNormal;
  secondLap.heading = spawn.heading;
  secondLap.orientationWxyz = spawn.orientationWxyz;
  secondLap.speed = 0.0F;
  secondLap.horizontalVelocityX = 0.0F;
  secondLap.horizontalVelocityZ = 0.0F;
  secondLap.gear = 1;
  secondLap.engineRpm = 0.0F;
  secondLap.engineTargetRpm = 0.0F;
  secondLap.gearShiftSeconds = 0.0F;
  secondLap.verticalVelocity = 0.0F;
  secondLap.grounded = true;
  if (!lapSimulation.restore(secondLap)) {
    std::cerr << "valid second-lap state was not restored\n";
    return 1;
  }
  tmnf::gameplay::VehicleState invalidLap = secondLap;
  invalidLap.completedLaps = 2;
  if (lapSimulation.restore(invalidLap)) {
    std::cerr << "unfinished state was accepted at the configured lap count\n";
    return 1;
  }
  for (int frame = 0; frame < 210; ++frame) {
    lapSimulation.update(1.0F / 60.0F, {.throttle = 1.0F});
  }
  if (!lapSimulation.state().finished || lapSimulation.state().completedLaps != 2U ||
      lapSimulation.state().checkpointCount != 1U) {
    std::cerr << "lap race did not finish after the configured lap count\n";
    return 1;
  }

  const tmnf::content::TrackScene wallScene = makeWallScene();
  const tmnf::content::TrackCollisionBuildResult wallCollision =
      tmnf::content::buildTrackCollision(wallScene);
  tmnf::gameplay::VehicleSimulation wallSimulation;
  if (!wallCollision ||
      !wallSimulation.initialize(wallScene, wallCollision.collision, tuning,
                                 &vehicleSolid)) {
    std::cerr << "wall collision fixture did not initialize\n";
    return 1;
  }
  for (int frame = 0; frame < 180; ++frame) {
    wallSimulation.update(1.0F / 60.0F, {.throttle = 1.0F});
  }
  if (!wallSimulation.state().grounded || wallSimulation.state().position.z > 55.11F ||
      wallSimulation.state().speed > 1.0F ||
      std::hypot(wallSimulation.state().horizontalVelocityX,
                 wallSimulation.state().horizontalVelocityZ) > 1.0F) {
    std::cerr << "vehicle passed through the fixture wall\n";
    return 1;
  }

  tmnf::gameplay::VehicleState bodyGeometry = spawn;
  bodyGeometry.position = {52.0F, 1.0F, 54.5F};
  bodyGeometry.orientationWxyz = {1.0F, 0.0F, 0.0F, 0.0F};
  bodyGeometry.heading = 0.0F;
  bodyGeometry.speed = 0.0F;
  bodyGeometry.horizontalVelocityX = 0.0F;
  bodyGeometry.horizontalVelocityZ = 0.0F;
  bodyGeometry.verticalVelocity = 0.0F;
  bodyGeometry.engineRpm = 0.0F;
  bodyGeometry.engineTargetRpm = 0.0F;
  bodyGeometry.grounded = false;
  if (!wallSimulation.restore(bodyGeometry)) {
    std::cerr << "body geometry state could not be restored\n";
    return 1;
  }
  wallSimulation.update(kStepSeconds, {});
  if (wallSimulation.state().position.z >= 54.45F) {
    std::cerr << "authored forward body ellipsoids did not reach the wall\n";
    return 1;
  }
  bodyGeometry.orientationWxyz = {0.707106781F, 0.0F, 0.707106781F, 0.0F};
  bodyGeometry.heading = 0.5F * 3.14159265358979323846F;
  if (!wallSimulation.restore(bodyGeometry)) {
    std::cerr << "yawed body geometry state could not be restored\n";
    return 1;
  }
  wallSimulation.update(kStepSeconds, {});
  if (std::abs(wallSimulation.state().position.z - bodyGeometry.position.z) > 1.0e-5F) {
    std::cerr << "yawed authored body geometry retained the old sphere extent\n";
    return 1;
  }

  tmnf::content::TrackScene nonCollidingWallScene = wallScene;
  nonCollidingWallScene.models[0].solid.surfaceMaterials = {13};
  const auto nonCollidingWall =
      tmnf::content::buildTrackCollision(nonCollidingWallScene);
  tmnf::gameplay::VehicleSimulation nonCollidingWallSimulation;
  bodyGeometry.orientationWxyz = {1.0F, 0.0F, 0.0F, 0.0F};
  bodyGeometry.heading = 0.0F;
  if (!nonCollidingWall ||
      !nonCollidingWallSimulation.initialize(nonCollidingWallScene,
                                             nonCollidingWall.collision, tuning,
                                             &vehicleSolid) ||
      !nonCollidingWallSimulation.restore(bodyGeometry)) {
    std::cerr << "non-colliding surface fixture did not initialize\n";
    return 1;
  }
  nonCollidingWallSimulation.update(kStepSeconds, {});
  if (std::abs(nonCollidingWallSimulation.state().position.z -
               bodyGeometry.position.z) > 1.0e-5F) {
    std::cerr << "surface 13 did not zero body depenetration\n";
    return 1;
  }

  for (const std::int16_t surfaceId : {std::int16_t{13}, std::int16_t{23}}) {
    tmnf::content::TrackScene nonSupportingScene = makeScene();
    nonSupportingScene.models[0].solid.surfaceMaterials = {surfaceId};
    const auto nonSupportingCollision =
        tmnf::content::buildTrackCollision(nonSupportingScene);
    tmnf::gameplay::VehicleSimulation nonSupportingSimulation;
    if (!nonSupportingCollision ||
        !nonSupportingSimulation.initialize(nonSupportingScene,
                                            nonSupportingCollision.collision, tuning)) {
      std::cerr << "non-supporting wheel surface fixture did not initialize\n";
      return 1;
    }
    nonSupportingSimulation.update(kStepSeconds, {});
    const bool hasWheelSupport = std::ranges::any_of(
        nonSupportingSimulation.state().wheels,
        [](const auto& wheel) { return wheel.contact; });
    if (nonSupportingSimulation.state().grounded || hasWheelSupport) {
      std::cerr << "surface " << surfaceId
                << " reached the source-rejected wheel callback\n";
      return 1;
    }
  }

  const tmnf::gbx::Float3 centerOfMass{
      (-0.884999990F + 0.885002017F) * 0.5F,
      (0.352499992F + 0.352499992F + 0.352503985F + 0.352503985F) * 0.25F -
          0.364F + tuning.centerOfMassDownUp,
      (-1.20550203F + 1.78208899F) * 0.5F};
  const float compensatedVerticalVelocity =
      10.0F * tuning.gravityCoefAir * kStepSeconds /
      (1.0F - tuning.linearFluidFrictionCoef * kStepSeconds);
  const float preContactSpeed =
      10.0F * (1.0F - tuning.linearFluidFrictionCoef * kStepSeconds);
  tmnf::gameplay::VehicleState bodyImpulse = bodyGeometry;
  bodyImpulse.position = {52.0F, 1.0F, 56.0F - centerOfMass.z - 0.05F};
  bodyImpulse.speed = 10.0F;
  bodyImpulse.horizontalVelocityZ = 10.0F;
  bodyImpulse.verticalVelocity = compensatedVerticalVelocity;
  bodyImpulse.engineRpm = 3'700.0F;
  bodyImpulse.engineTargetRpm = 3'700.0F;
  const tmnf::gbx::SolidModel centeredBody =
      makeImpulseCollisionSolid(centerOfMass, 0.0F);
  tmnf::gameplay::VehicleSimulation centeredImpulseSimulation;
  if (!centeredImpulseSimulation.initialize(wallScene, wallCollision.collision, tuning,
                                            &centeredBody) ||
      !centeredImpulseSimulation.restore(bodyImpulse)) {
    std::cerr << "centered body impulse fixture did not initialize\n";
    return 1;
  }
  centeredImpulseSimulation.update(kStepSeconds, {});
  if (std::abs(centeredImpulseSimulation.state().horizontalVelocityZ -
               0.5F * preContactSpeed) > 1.0e-4F ||
      std::abs(centeredImpulseSimulation.state().angularVelocity.y) > 1.0e-5F) {
    std::cerr << "centered body restitution impulse mismatch: vz="
              << centeredImpulseSimulation.state().horizontalVelocityZ << ", wy="
              << centeredImpulseSimulation.state().angularVelocity.y << '\n';
    return 1;
  }

  const tmnf::gbx::SolidModel offsetBody =
      makeImpulseCollisionSolid(centerOfMass, 1.0F);
  tmnf::gameplay::VehicleSimulation offsetImpulseSimulation;
  if (!offsetImpulseSimulation.initialize(wallScene, wallCollision.collision, tuning,
                                          &offsetBody) ||
      !offsetImpulseSimulation.restore(bodyImpulse)) {
    std::cerr << "offset body impulse fixture did not initialize\n";
    return 1;
  }
  offsetImpulseSimulation.update(kStepSeconds, {});
  constexpr float kBodyInverseInertiaY = 0.48F;
  const float expectedOffsetImpulse =
      0.5F * preContactSpeed / (1.0F + kBodyInverseInertiaY);
  if (std::abs(offsetImpulseSimulation.state().horizontalVelocityZ -
               (preContactSpeed - expectedOffsetImpulse)) > 1.0e-4F ||
      std::abs(offsetImpulseSimulation.state().angularVelocity.y -
               expectedOffsetImpulse * kBodyInverseInertiaY *
                   tuning.angularSpeedYImpulseScale) > 1.0e-4F) {
    std::cerr << "offset body effective-mass impulse mismatch: vz="
              << offsetImpulseSimulation.state().horizontalVelocityZ << ", wy="
              << offsetImpulseSimulation.state().angularVelocity.y << '\n';
    return 1;
  }
  return 0;
}
