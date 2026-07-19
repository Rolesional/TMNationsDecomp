#include "tmnf/gameplay/race3_camera.hpp"

#include <algorithm>
#include <array>
#include <cmath>

namespace tmnf::gameplay {
namespace {

gbx::Float3 add(const gbx::Float3 left, const gbx::Float3 right) noexcept {
  return {left.x + right.x, left.y + right.y, left.z + right.z};
}

gbx::Float3 multiply(const gbx::Float3 value, const float scale) noexcept {
  return {value.x * scale, value.y * scale, value.z * scale};
}

float dot(const gbx::Float3 left, const gbx::Float3 right) noexcept {
  return left.x * right.x + left.y * right.y + left.z * right.z;
}

gbx::Float3 normalize(const gbx::Float3 value) noexcept {
  const float lengthSquared = dot(value, value);
  if (lengthSquared <= 1.0e-10F || !std::isfinite(lengthSquared)) {
    return {0.0F, 1.0F, 0.0F};
  }
  return multiply(value, 1.0F / std::sqrt(lengthSquared));
}

float speedGain(const float signedSpeedKmh) noexcept {
  constexpr std::array<float, 6> x{-100.0F, -50.0F, 0.0F, 250.0F, 500.0F, 700.0F};
  constexpr std::array<float, 6> y{1.5F, 1.2F, 1.0F, 1.5F, 2.0F, 3.0F};
  if (signedSpeedKmh <= x.front()) {
    return y.front();
  }
  if (signedSpeedKmh >= x.back()) {
    return y.back();
  }
  const auto right = std::upper_bound(x.begin(), x.end(), signedSpeedKmh);
  const std::size_t rightIndex = static_cast<std::size_t>(right - x.begin());
  const std::size_t leftIndex = rightIndex - 1U;
  return std::lerp(y[leftIndex], y[rightIndex],
                   (signedSpeedKmh - x[leftIndex]) / (x[rightIndex] - x[leftIndex]));
}

gbx::Float3 sphericalMix(const gbx::Float3 from, const gbx::Float3 to,
                         const float fraction) noexcept {
  const float cosine = std::clamp(dot(from, to), -1.0F, 1.0F);
  if (1.0F - cosine <= 1.0e-5F) {
    return add(multiply(from, 1.0F - fraction), multiply(to, fraction));
  }
  const float angle = std::acos(cosine);
  const float sine = std::sin(angle);
  return add(multiply(from, std::sin((1.0F - fraction) * angle) / sine),
             multiply(to, std::sin(fraction * angle) / sine));
}

float followFraction(const gbx::Float3 current, const gbx::Float3 desired, const float gain,
                     const float deltaSeconds, const float speedMultiplier) noexcept {
  const float angle = std::acos(std::clamp(dot(current, desired), -1.0F, 1.0F));
  return speedMultiplier *
         std::clamp(gain * deltaSeconds / (2.0F * angle + 1.0F), 0.0F, 1.0F);
}

} // namespace

Race3CameraFrame Race3Camera::update(const gbx::Float3 vehicleUp,
                                     const gbx::Float3 vehicleForward,
                                     const float signedForwardSpeedMetersPerSecond,
                                     const double seconds, const bool reset) noexcept {
  constexpr float kHorizontalDistance = 4.0F;
  constexpr float kVerticalDistance = 2.0F;
  constexpr float kTargetHeight = 1.0F;
  constexpr float kDistance = 4.472135954999579F; // sqrt(4^2 + 2^2)
  const gbx::Float3 desiredUp = normalize(vehicleUp);
  const gbx::Float3 desiredDirection = normalize(
      add(multiply(desiredUp, kVerticalDistance),
          multiply(normalize(vehicleForward), -kHorizontalDistance)));

  if (!initialized_ || reset) {
    direction_ = desiredDirection;
    up_ = desiredUp;
    distance_ = kDistance;
    distanceVelocity_ = 0.0F;
    initialized_ = true;
  } else {
    const float deltaSeconds = static_cast<float>(
        std::clamp(std::isfinite(seconds) ? seconds - lastSeconds_ : 0.0, 0.0, 0.2));
    distanceVelocity_ +=
        deltaSeconds * ((kDistance - distance_) * 10.0F - distanceVelocity_ * 5.0F);
    distance_ += deltaSeconds * distanceVelocity_;
    const float multiplier = speedGain(
        std::isfinite(signedForwardSpeedMetersPerSecond)
            ? signedForwardSpeedMetersPerSecond * 3.6F
            : 0.0F);
    const float directionFraction =
        followFraction(direction_, desiredDirection, 5.0F, deltaSeconds, multiplier);
    direction_ = sphericalMix(direction_, desiredDirection, directionFraction);
    const float upFraction = followFraction(up_, desiredUp, 4.0F, deltaSeconds, multiplier);
    up_ = sphericalMix(up_, desiredUp, upFraction);
  }
  lastSeconds_ = std::isfinite(seconds) ? seconds : lastSeconds_;
  constexpr float kOrdinaryVerticalFieldOfViewDegrees = 90.0F;
  return {multiply(direction_, distance_), multiply(up_, kTargetHeight), up_,
          kOrdinaryVerticalFieldOfViewDegrees};
}

void Race3Camera::clear() noexcept {
  distance_ = 4.47213595F;
  distanceVelocity_ = 0.0F;
  lastSeconds_ = 0.0;
  initialized_ = false;
}

} // namespace tmnf::gameplay
