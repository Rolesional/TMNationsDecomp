#include "tmnf/content/track_collision.hpp"

#include <cmath>
#include <iostream>

namespace {

bool near(const float left, const float right) { return std::abs(left - right) < 1.0e-5F; }

tmnf::content::TrackScene makeFixture() {
  tmnf::content::TrackScene scene;
  scene.challenge.size = {4, 4, 4};
  tmnf::gbx::ChallengeBlock block;
  block.name = "Fixture";
  block.coordinate = {1, 2, 1};
  scene.challenge.blocks.push_back(block);

  tmnf::content::TrackModel model;
  model.logicalPath = "Fixture.Solid.Gbx";
  tmnf::gbx::SolidCollisionShape shape;
  shape.type = tmnf::gbx::SolidCollisionType::Mesh;
  shape.archiveVersion = 3;
  shape.positions = {{0.0F, 0.0F, 0.0F}, {32.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 32.0F}};
  shape.indices = {0, 2, 1};
  shape.triangleSurfaceIndices = {0};
  shape.surfaceId = 0;
  model.solid.collisionShapes.push_back(std::move(shape));
  model.solid.surfaceMaterials.push_back(16);
  scene.models.push_back(std::move(model));
  scene.placements.push_back({0, 0});

  tmnf::content::TrackTrigger trigger;
  trigger.blockIndex = 0;
  trigger.logicalPath = "FixtureTrigger.Solid.Gbx";
  tmnf::gbx::SolidCollisionShape triggerShape;
  triggerShape.type = tmnf::gbx::SolidCollisionType::Mesh;
  triggerShape.positions = {
      {0.0F, 0.0F, 16.0F}, {32.0F, 0.0F, 16.0F}, {32.0F, 4.0F, 16.0F}, {0.0F, 4.0F, 16.0F}};
  triggerShape.indices = {0, 1, 2, 0, 2, 3};
  trigger.solid.collisionShapes.push_back(std::move(triggerShape));
  scene.triggers.push_back(std::move(trigger));
  return scene;
}

} // namespace

int main() {
  tmnf::content::TrackScene scene = makeFixture();
  const tmnf::content::TrackCollisionBuildResult built = tmnf::content::buildTrackCollision(scene);
  if (!built) {
    std::cerr << built.error << '\n';
    return 1;
  }
  if (built.collision.positions.size() != 3 || built.collision.indices.size() != 3 ||
      built.collision.triangleSurfaceIds != std::vector<std::int16_t>{16} ||
      built.collision.triggers.size() != 1U ||
      built.collision.triggers.front().indices.size() != 6U) {
    std::cerr << "instanced collision mesh is incorrect\n";
    return 1;
  }
  const tmnf::content::TrackTriggerMesh& trigger = built.collision.triggers.front();
  if (!tmnf::content::segmentIntersectsTrackTrigger(trigger, {40.0F, 18.0F, 40.0F},
                                                    {40.0F, 18.0F, 56.0F}) ||
      !tmnf::content::segmentIntersectsTrackTrigger(trigger, {40.0F, 18.0F, 56.0F},
                                                    {40.0F, 18.0F, 40.0F}) ||
      tmnf::content::segmentIntersectsTrackTrigger(trigger, {70.0F, 18.0F, 40.0F},
                                                   {70.0F, 18.0F, 56.0F})) {
    std::cerr << "two-sided trigger segment query is incorrect\n";
    return 1;
  }
  const std::optional<tmnf::content::TrackGroundHit> hit =
      tmnf::content::sampleTrackGround(built.collision, 40.0F, 40.0F, 0.0F, 32.0F);
  if (!hit.has_value() || !near(hit->height, 16.0F) || !near(hit->normal.y, 1.0F) ||
      hit->surfaceId != 16) {
    std::cerr << "ground query did not hit the fixture triangle\n";
    return 1;
  }
  if (tmnf::content::sampleTrackGround(built.collision, 60.0F, 60.0F, 0.0F, 32.0F)) {
    std::cerr << "ground query hit outside the fixture triangle\n";
    return 1;
  }

  tmnf::content::TrackCollision wall;
  wall.gridWidth = 1;
  wall.gridDepth = 1;
  wall.positions = {{0.0F, 0.0F, 0.0F}, {0.0F, 2.0F, 0.0F}, {0.0F, 0.0F, 2.0F}};
  wall.indices = {0, 1, 2};
  wall.triangleSurfaceIds = {23};
  wall.cellTriangles = {{0}};
  const std::optional<tmnf::content::TrackWallContact> wallHit =
      tmnf::content::sampleTrackWallContact(wall, {0.5F, 0.5F, 0.5F}, 1.0F);
  if (!wallHit.has_value() || !near(wallHit->penetration, 0.5F) || !near(wallHit->normal.x, 1.0F) ||
      !near(wallHit->normal.y, 0.0F) || wallHit->surfaceId != 23) {
    std::cerr << "wall contact query did not resolve the fixture plane\n";
    return 1;
  }
  if (tmnf::content::sampleTrackWallContact(wall, {2.0F, 0.5F, 0.5F}, 0.5F)) {
    std::cerr << "wall contact query hit outside the fixture radius\n";
    return 1;
  }

  tmnf::gbx::ChallengeBlock rotated;
  rotated.coordinate = {1, 0, 1};
  rotated.direction = 1;
  const tmnf::gbx::Float3 placed = tmnf::content::placeTrackPoint({0.0F, 0.0F, 0.0F}, rotated);
  if (!near(placed.x, 64.0F) || !near(placed.z, 32.0F)) {
    std::cerr << "block placement rotation is incorrect\n";
    return 1;
  }

  scene.models[0].solid.collisionShapes[0].indices[2] = 3;
  if (tmnf::content::buildTrackCollision(scene)) {
    std::cerr << "out-of-range collision index was accepted\n";
    return 1;
  }
  return 0;
}
