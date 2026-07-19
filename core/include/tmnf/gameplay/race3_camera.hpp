#pragma once

#include "tmnf/gbx/solid.hpp"

namespace tmnf::gameplay {

struct Race3CameraFrame final {
  gbx::Float3 eyeOffset;
  gbx::Float3 targetOffset;
  gbx::Float3 up{0.0F, 1.0F, 0.0F};
  float verticalFieldOfViewDegrees = 90.0F;
};

// Source-owned neutral chase behavior from CGameControlCameraTrackManiaRace3.
// Offsets are returned in vehicle/world units and can be added to the vehicle pivot.
class Race3Camera final {
public:
  [[nodiscard]] Race3CameraFrame update(gbx::Float3 vehicleUp, gbx::Float3 vehicleForward,
                                        float signedForwardSpeedMetersPerSecond, double seconds,
                                        bool reset) noexcept;
  void clear() noexcept;

private:
  gbx::Float3 direction_{0.0F, 0.4472136F, -0.8944272F};
  gbx::Float3 up_{0.0F, 1.0F, 0.0F};
  float distance_ = 4.47213595F;
  float distanceVelocity_ = 0.0F;
  double lastSeconds_ = 0.0;
  bool initialized_ = false;
};

} // namespace tmnf::gameplay
