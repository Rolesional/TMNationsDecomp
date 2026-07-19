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
  shape.positions = {{0.0F, 0.0F, 0.0F}, {32.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 32.0F},
                     {0.0F, 4.0F, 0.0F}, {32.0F, 4.0F, 0.0F}, {0.0F, 4.0F, 32.0F}};
  shape.indices = {0, 2, 1, 3, 5, 4};
  shape.triangleSurfaceIndices = {0, 1};
  shape.surfaceId = 0;
  shape.surfaceMaterialRange = std::array<std::size_t, 2>{0, 2};
  model.solid.collisionShapes.push_back(std::move(shape));
  tmnf::gbx::SolidCollisionShape fallbackNotCollidable;
  fallbackNotCollidable.type = tmnf::gbx::SolidCollisionType::Mesh;
  fallbackNotCollidable.archiveVersion = 3;
  fallbackNotCollidable.positions = {
      {0.0F, 8.0F, 0.0F}, {32.0F, 8.0F, 0.0F}, {0.0F, 8.0F, 32.0F}};
  fallbackNotCollidable.indices = {0, 2, 1};
  fallbackNotCollidable.triangleSurfaceIndices = {-1};
  fallbackNotCollidable.surfaceId = 28;
  model.solid.collisionShapes.push_back(std::move(fallbackNotCollidable));
  tmnf::gbx::SolidCollisionShape secondTable;
  secondTable.type = tmnf::gbx::SolidCollisionType::Mesh;
  secondTable.archiveVersion = 3;
  secondTable.positions = {
      {64.0F, 0.0F, 0.0F}, {96.0F, 0.0F, 0.0F}, {64.0F, 0.0F, 32.0F}};
  secondTable.indices = {0, 2, 1};
  secondTable.triangleSurfaceIndices = {0};
  secondTable.surfaceId = 0;
  secondTable.surfaceMaterialRange = std::array<std::size_t, 2>{2, 1};
  model.solid.collisionShapes.push_back(std::move(secondTable));
  model.solid.surfaceMaterials = {16, 28, 4};
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
  if (built.collision.positions.size() != 12 || built.collision.indices.size() != 6 ||
      built.collision.triangleSurfaceIds != std::vector<std::int16_t>{16, 4} ||
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
  const auto secondTableHit =
      tmnf::content::sampleTrackGround(built.collision, 104.0F, 40.0F, 0.0F, 32.0F);
  if (!secondTableHit.has_value() || secondTableHit->surfaceId != 4 ||
      !near(secondTableHit->height, 16.0F)) {
    std::cerr << "per-surface local material table was flattened\n";
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
  if (tmnf::content::sampleTrackWallContact(wall, {0.5F, 0.5F, 0.5F}, 1.0F, 2.1F)) {
    std::cerr << "wall contact query did not honor the minimum contact height\n";
    return 1;
  }

  tmnf::content::TrackCollision ellipsoidTrack;
  ellipsoidTrack.gridWidth = 1;
  ellipsoidTrack.gridDepth = 1;
  ellipsoidTrack.positions = {{0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 2.0F},
                              {2.0F, 0.0F, 0.0F}};
  ellipsoidTrack.indices = {0, 1, 2};
  ellipsoidTrack.triangleSurfaceIds = {42};
  ellipsoidTrack.cellTriangles = {{0}};
  constexpr std::array<float, 9> identity{1.0F, 0.0F, 0.0F, 0.0F, 1.0F,
                                          0.0F, 0.0F, 0.0F, 1.0F};
  const auto faceContact = tmnf::content::sampleTrackEllipsoidContact(
      ellipsoidTrack, {0.5F, 0.3F, 0.5F}, {0.2F, 0.5F, 0.5F}, identity,
      {0.0F, 1.0F, 0.0F});
  if (!faceContact.has_value() || !near(faceContact->axisCorrection, 0.2F) ||
      !near(faceContact->correction.y, 0.2F) || !near(faceContact->normal.y, 1.0F) ||
      !near(faceContact->point.y, 0.0F) || faceContact->surfaceId != 42) {
    std::cerr << "ellipsoid face correction does not match 008C1C70\n";
    return 1;
  }
  const auto touchingContact = tmnf::content::sampleTrackEllipsoidContact(
      ellipsoidTrack, {0.5F, 0.5F, 0.5F}, {0.2F, 0.5F, 0.5F}, identity,
      {0.0F, 1.0F, 0.0F});
  if (!touchingContact.has_value() || !near(touchingContact->axisCorrection, 0.0F) ||
      !near(touchingContact->correction.y, 0.0F)) {
    std::cerr << "source zero-correction ellipsoid contact was dropped\n";
    return 1;
  }
  const auto edgeContact = tmnf::content::sampleTrackEllipsoidContact(
      ellipsoidTrack, {1.2F, 0.2F, 1.2F}, {0.5F, 0.5F, 0.5F}, identity,
      {0.0F, 1.0F, 0.0F});
  if (!edgeContact.has_value() || !near(edgeContact->axisCorrection, 0.0886751F) ||
      !near(edgeContact->point.x, 1.0F) || !near(edgeContact->point.y, 0.0F) ||
      !near(edgeContact->point.z, 1.0F) || edgeContact->normal.y <= 0.5F) {
    std::cerr << "ellipsoid edge correction does not match 008C1C70\n";
    return 1;
  }
  constexpr std::array<float, 9> quarterTurnZ{0.0F, -1.0F, 0.0F, 1.0F, 0.0F,
                                              0.0F, 0.0F, 0.0F, 1.0F};
  const auto rotatedContact = tmnf::content::sampleTrackEllipsoidContact(
      ellipsoidTrack, {0.5F, 0.1F, 0.5F}, {0.2F, 0.5F, 0.5F}, quarterTurnZ,
      {0.0F, 1.0F, 0.0F});
  if (!rotatedContact.has_value() || !near(rotatedContact->axisCorrection, 0.1F) ||
      !near(rotatedContact->normal.y, 1.0F)) {
    std::cerr << "rotated nonuniform ellipsoid correction is incorrect\n";
    return 1;
  }
  tmnf::content::TrackCollision enumeratedTrack = ellipsoidTrack;
  enumeratedTrack.indices.insert(enumeratedTrack.indices.end(), {0, 1, 2});
  enumeratedTrack.triangleSurfaceIds.push_back(43);
  enumeratedTrack.cellTriangles = {{0, 1, 0}};
  std::vector<tmnf::content::TrackEllipsoidContact> enumeratedContacts;
  tmnf::content::appendTrackEllipsoidContacts(
      enumeratedTrack, {0.5F, 0.3F, 0.5F}, {0.2F, 0.5F, 0.5F}, identity,
      enumeratedContacts);
  if (enumeratedContacts.size() != 2U || enumeratedContacts[0].triangleIndex != 0U ||
      enumeratedContacts[1].triangleIndex != 1U ||
      enumeratedContacts[0].surfaceId != 42 || enumeratedContacts[1].surfaceId != 43) {
    std::cerr << "ellipsoid contact enumeration/deduplication mismatch\n";
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
  const tmnf::gbx::Float3 placedTwoCell =
      tmnf::content::placeTrackPoint({0.0F, 0.0F, 0.0F}, rotated, {2, 2});
  if (!near(placedTwoCell.x, 96.0F) || !near(placedTwoCell.z, 32.0F)) {
    std::cerr << "multi-cell block placement rotation is incorrect\n";
    return 1;
  }

  scene.models[0].solid.collisionShapes[0].indices[2] = 6;
  if (tmnf::content::buildTrackCollision(scene)) {
    std::cerr << "out-of-range collision index was accepted\n";
    return 1;
  }
  return 0;
}
