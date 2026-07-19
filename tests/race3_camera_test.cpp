#include "tmnf/gameplay/race3_camera.hpp"

#include <cmath>
#include <iostream>

namespace {

bool near(const float left, const float right, const float epsilon = 1.0e-5F) {
  return std::abs(left - right) <= epsilon;
}

bool near(const tmnf::gbx::Float3 left, const tmnf::gbx::Float3 right,
          const float epsilon = 1.0e-5F) {
  return near(left.x, right.x, epsilon) && near(left.y, right.y, epsilon) &&
         near(left.z, right.z, epsilon);
}

tmnf::gbx::Float3 rotateX(const tmnf::gbx::Float3 value, const float angle) {
  const float sine = std::sin(angle);
  const float cosine = std::cos(angle);
  return {value.x, value.y * cosine - value.z * sine,
          value.y * sine + value.z * cosine};
}

} // namespace

int main() {
  constexpr tmnf::gbx::Float3 up{0.0F, 1.0F, 0.0F};
  constexpr tmnf::gbx::Float3 forward{0.0F, 0.0F, 1.0F};
  tmnf::gameplay::Race3Camera camera;
  auto frame = camera.update(up, forward, 0.0F, 0.0, true);
  if (!near(frame.eyeOffset, {0.0F, 2.0F, -4.0F}) ||
      !near(frame.targetOffset, {0.0F, 1.0F, 0.0F}) || !near(frame.up, up) ||
      !near(frame.verticalFieldOfViewDegrees, 90.0F)) {
    std::cerr << "Race3 reset geometry is incorrect\n";
    return 1;
  }

  constexpr float angle = 0.2F;
  const tmnf::gbx::Float3 rolledUp = rotateX(up, angle);
  const tmnf::gbx::Float3 rolledForward = rotateX(forward, angle);
  frame = camera.update(rolledUp, rolledForward, 0.0F, 1.0 / 60.0, false);
  constexpr float directionFraction = (5.0F / 60.0F) / 1.4F;
  constexpr float upFraction = (4.0F / 60.0F) / 1.4F;
  const tmnf::gbx::Float3 expectedDirection =
      rotateX({0.0F, 0.4472135955F, -0.894427191F}, angle * directionFraction);
  if (!near(frame.eyeOffset,
            {expectedDirection.x * std::sqrt(20.0F),
             expectedDirection.y * std::sqrt(20.0F),
             expectedDirection.z * std::sqrt(20.0F)}) ||
      !near(frame.up, rotateX(up, angle * upFraction))) {
    std::cerr << "Race3 ordinary loose-follow gains are incorrect\n";
    return 1;
  }

  camera.clear();
  frame = camera.update(up, forward, 0.0F, 0.0, true);
  frame = camera.update(rolledUp, rolledForward, 250.0F / 3.6F, 1.0 / 60.0, false);
  const tmnf::gbx::Float3 speedDirection =
      rotateX({0.0F, 0.4472135955F, -0.894427191F}, angle * directionFraction * 1.5F);
  if (!near(frame.eyeOffset,
            {speedDirection.x * std::sqrt(20.0F), speedDirection.y * std::sqrt(20.0F),
             speedDirection.z * std::sqrt(20.0F)}) ||
      !near(frame.up, rotateX(up, angle * upFraction * 1.5F))) {
    std::cerr << "Race3 signed-speed follow gain is incorrect\n";
    return 1;
  }

  camera.clear();
  frame = camera.update(up, forward, 0.0F, 10.0, true);
  const auto backwards = camera.update(rolledUp, rolledForward, 0.0F, 9.0, false);
  if (!near(backwards.eyeOffset, {0.0F, 2.0F, -4.0F}) || !near(backwards.up, up)) {
    std::cerr << "Race3 backwards clock was not clamped to zero\n";
    return 1;
  }
  const auto snapped = camera.update(rolledUp, rolledForward, 0.0F, 9.1, true);
  if (!near(snapped.eyeOffset, rotateX({0.0F, 2.0F, -4.0F}, angle)) ||
      !near(snapped.targetOffset, rolledUp) || !near(snapped.up, rolledUp)) {
    std::cerr << "Race3 explicit reset did not snap to the vehicle basis\n";
    return 1;
  }

  return 0;
}
