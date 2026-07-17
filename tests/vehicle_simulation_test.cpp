#include "tmnf/content/track_collision.hpp"
#include "tmnf/gameplay/vehicle.hpp"

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
  wall.indices = {0, 1, 2, 0, 2, 3};
  wall.triangleSurfaceIndices = {0, 0};
  scene.models[0].solid.collisionShapes.push_back(std::move(wall));
  return scene;
}

} // namespace

int main() {
  const tmnf::content::TrackScene scene = makeScene();
  const tmnf::content::TrackCollisionBuildResult collision =
      tmnf::content::buildTrackCollision(scene);
  if (!collision) {
    std::cerr << collision.error << '\n';
    return 1;
  }
  tmnf::gameplay::VehicleSimulation simulation;
  const tmnf::gameplay::VehicleInitializeResult initialized =
      simulation.initialize(scene, collision.collision);
  if (!initialized) {
    std::cerr << initialized.error << '\n';
    return 1;
  }
  const tmnf::gameplay::VehicleState spawn = simulation.state();
  if (!spawn.grounded || std::abs(spawn.position.x - 52.0F) > 1.0e-5F ||
      std::abs(spawn.position.y - 0.3F) > 1.0e-5F || std::abs(spawn.position.z - 44.0F) > 1.0e-5F) {
    std::cerr << "vehicle spawn state is incorrect\n";
    return 1;
  }
  for (int frame = 0; frame < 60; ++frame) {
    simulation.update(1.0F / 60.0F, {.throttle = 1.0F});
  }
  const tmnf::gameplay::VehicleState accelerated = simulation.state();
  if (!accelerated.grounded || accelerated.speed < 20.0F ||
      std::abs(accelerated.position.z - spawn.position.z) <= 8.0F) {
    std::cerr << "vehicle did not accelerate over the collision surface\n";
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
  invalid.completedLaps = 1;
  if (simulation.restore(invalid)) {
    std::cerr << "lap progress was accepted for a point-to-point race\n";
    return 1;
  }

  tmnf::gameplay::VehicleSimulation raceSimulation;
  if (!raceSimulation.initialize(scene, collision.collision) ||
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
      raceSimulation.state().raceTimeSeconds != checkpointRaceTime) {
    std::cerr << "checkpoint respawn did not preserve race progress\n";
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
      !noRespawnSimulation.initialize(noRespawnScene, noRespawnCollision.collision)) {
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
  if (!lapCollision || !lapSimulation.initialize(lapScene, lapCollision.collision) ||
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
  secondLap.speed = 0.0F;
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
  if (!wallCollision || !wallSimulation.initialize(wallScene, wallCollision.collision)) {
    std::cerr << "wall collision fixture did not initialize\n";
    return 1;
  }
  for (int frame = 0; frame < 180; ++frame) {
    wallSimulation.update(1.0F / 60.0F, {.throttle = 1.0F});
  }
  if (!wallSimulation.state().grounded || wallSimulation.state().position.z > 55.11F ||
      wallSimulation.state().speed > 1.0F) {
    std::cerr << "vehicle passed through the fixture wall\n";
    return 1;
  }
  return 0;
}
