#include "tmnf/content/block_assets.hpp"

#include <iostream>

int main() {
  const auto circuit = tmnf::content::resolveReferencePath(
      "Stadium\\ConstructionBlockInfo\\ConstructionBlockInfoClassic\\StadiumCircuitBase."
      "TMEDClassic.Gbx",
      2, "Media\\Solid\\Circuit\\Road\\BaseGround.Solid.Gbx");
  if (circuit != "Stadium\\Media\\Solid\\Circuit\\Road\\BaseGround.Solid.Gbx") {
    std::cerr << "BlockInfo reference ancestry resolved incorrectly\n";
    return 1;
  }
  const auto mobil = tmnf::content::resolveReferencePath(
      "Stadium\\Mobil\\StadiumPlatformCheckpointAir.Mobil.Gbx", 2,
      "Stadium\\Media\\Solid\\Platform\\Checkpoint\\Air.Solid.Gbx");
  if (mobil != "Stadium\\Media\\Solid\\Platform\\Checkpoint\\Air.Solid.Gbx") {
    std::cerr << "Mobil reference ancestry resolved incorrectly\n";
    return 1;
  }
  const auto parent = tmnf::content::resolveReferencePath(
      "Stadium\\Media\\One\\Source.Gbx", 0, "..\\Two\\Target.Gbx");
  if (parent != "Stadium\\Media\\Two\\Target.Gbx") {
    std::cerr << "relative parent reference resolved incorrectly\n";
    return 1;
  }
  if (tmnf::content::resolveReferencePath("Source.Gbx", 1, "Target.Gbx").has_value()) {
    std::cerr << "invalid reference ancestry was accepted\n";
    return 1;
  }
  return 0;
}
