#include "tmnf/platform/input.hpp"

#include <algorithm>

namespace tmnf::platform {
namespace {

std::size_t indexOf(const InputAction action) noexcept { return static_cast<std::size_t>(action); }

bool valid(const InputAction action) noexcept { return indexOf(action) < kInputActionCount; }

} // namespace

bool InputSnapshot::isHeld(const InputAction action) const noexcept {
  return valid(action) && held[indexOf(action)];
}

bool InputSnapshot::wasPressed(const InputAction action) const noexcept {
  return valid(action) && pressed[indexOf(action)];
}

bool InputSnapshot::wasReleased(const InputAction action) const noexcept {
  return valid(action) && released[indexOf(action)];
}

void InputState::setDigital(const InputAction action, const bool down) noexcept {
  if (!valid(action)) {
    return;
  }
  const std::size_t index = indexOf(action);
  if (state_.held[index] == down) {
    return;
  }
  state_.held[index] = down;
  if (down) {
    state_.pressed[index] = true;
  } else {
    state_.released[index] = true;
  }
}

void InputState::setAnalog(const float steering, const float throttle, const float brake) noexcept {
  state_.steering = std::clamp(steering, -1.0F, 1.0F);
  state_.throttle = std::clamp(throttle, 0.0F, 1.0F);
  state_.brake = std::clamp(brake, 0.0F, 1.0F);
}

void InputState::setPrimaryTouch(const bool active, const std::int32_t id, const float x,
                                 const float y) noexcept {
  state_.touchActive = active;
  state_.touchId = active ? id : -1;
  state_.touchX = std::clamp(x, -1.0F, 1.0F);
  state_.touchY = std::clamp(y, -1.0F, 1.0F);
}

void InputState::setFocused(const bool focused) noexcept {
  state_.focused = focused;
  if (!focused) {
    releaseAll();
  }
}

void InputState::releaseAll() noexcept {
  for (std::size_t index = 0; index < kInputActionCount; ++index) {
    if (state_.held[index]) {
      state_.held[index] = false;
      state_.released[index] = true;
    }
  }
  state_.steering = 0.0F;
  state_.throttle = 0.0F;
  state_.brake = 0.0F;
  state_.touchActive = false;
  state_.touchId = -1;
}

const InputSnapshot& InputState::current() const noexcept { return state_; }

InputSnapshot InputState::consume() noexcept {
  InputSnapshot snapshot = state_;
  state_.pressed.fill(false);
  state_.released.fill(false);
  return snapshot;
}

} // namespace tmnf::platform
