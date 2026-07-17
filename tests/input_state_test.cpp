#include "tmnf/platform/input.hpp"

#include <iostream>

int main() {
  using tmnf::platform::InputAction;
  using tmnf::platform::InputSnapshot;
  using tmnf::platform::InputState;

  InputState input;
  input.setFocused(true);
  input.setDigital(InputAction::Accelerate, true);
  input.setAnalog(1.5F, 0.75F, -1.0F);
  input.setPrimaryTouch(true, 7, 2.0F, -2.0F);

  const InputSnapshot first = input.consume();
  if (!first.focused || !first.isHeld(InputAction::Accelerate) ||
      !first.wasPressed(InputAction::Accelerate) || first.wasReleased(InputAction::Accelerate) ||
      first.steering != 1.0F || first.throttle != 0.75F || first.brake != 0.0F ||
      !first.touchActive || first.touchId != 7 || first.touchX != 1.0F || first.touchY != -1.0F) {
    std::cerr << "initial input snapshot was incorrect\n";
    return 1;
  }

  const InputSnapshot held = input.consume();
  if (!held.isHeld(InputAction::Accelerate) || held.wasPressed(InputAction::Accelerate) ||
      held.wasReleased(InputAction::Accelerate)) {
    std::cerr << "input transitions did not clear after consumption\n";
    return 1;
  }

  input.setFocused(false);
  const InputSnapshot released = input.consume();
  if (released.focused || released.isHeld(InputAction::Accelerate) ||
      !released.wasReleased(InputAction::Accelerate) || released.touchActive ||
      released.steering != 0.0F || released.throttle != 0.0F || released.brake != 0.0F) {
    std::cerr << "focus loss did not release persistent input\n";
    return 1;
  }
  return 0;
}
