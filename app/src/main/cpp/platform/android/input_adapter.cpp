#include "platform/android/input_adapter.hpp"

#include <android/input.h>
#include <game-activity/GameActivityEvents.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>

#include <algorithm>
#include <cmath>
#include <optional>

namespace tmnf::android {
namespace {

using tmnf::platform::InputAction;

std::optional<InputAction> actionForKey(const std::int32_t keyCode) noexcept {
  switch (keyCode) {
  case AKEYCODE_W:
  case AKEYCODE_DPAD_UP:
  case AKEYCODE_BUTTON_R2:
    return InputAction::Accelerate;
  case AKEYCODE_S:
  case AKEYCODE_DPAD_DOWN:
  case AKEYCODE_BUTTON_L2:
    return InputAction::Brake;
  case AKEYCODE_A:
  case AKEYCODE_DPAD_LEFT:
    return InputAction::SteerLeft;
  case AKEYCODE_D:
  case AKEYCODE_DPAD_RIGHT:
    return InputAction::SteerRight;
  case AKEYCODE_ENTER:
  case AKEYCODE_SPACE:
  case AKEYCODE_DPAD_CENTER:
  case AKEYCODE_BUTTON_A:
    return InputAction::Confirm;
  case AKEYCODE_BACK:
  case AKEYCODE_ESCAPE:
  case AKEYCODE_BUTTON_B:
    return InputAction::Cancel;
  case AKEYCODE_R:
  case AKEYCODE_BUTTON_Y:
    return InputAction::Respawn;
  case AKEYCODE_P:
  case AKEYCODE_MENU:
  case AKEYCODE_BUTTON_START:
    return InputAction::Menu;
  default:
    return std::nullopt;
  }
}

float withDeadZone(const float value) noexcept {
  constexpr float kDeadZone = 0.12F;
  const float magnitude = std::abs(value);
  if (magnitude <= kDeadZone) {
    return 0.0F;
  }
  return std::copysign((magnitude - kDeadZone) / (1.0F - kDeadZone), value);
}

float normalizedX(const GameActivityPointerAxes& pointer, const int width) noexcept {
  if (width <= 0) {
    return 0.0F;
  }
  return std::clamp(GameActivityPointerAxes_getX(&pointer) * 2.0F / static_cast<float>(width) -
                        1.0F,
                    -1.0F, 1.0F);
}

float normalizedY(const GameActivityPointerAxes& pointer, const int height) noexcept {
  if (height <= 0) {
    return 0.0F;
  }
  return std::clamp(1.0F -
                        GameActivityPointerAxes_getY(&pointer) * 2.0F / static_cast<float>(height),
                    -1.0F, 1.0F);
}

} // namespace

InputAdapter::InputAdapter(tmnf::platform::InputState& input) : input_(input) {
  for (const std::int32_t axis :
       {AMOTION_EVENT_AXIS_X, AMOTION_EVENT_AXIS_Y, AMOTION_EVENT_AXIS_Z, AMOTION_EVENT_AXIS_RZ,
        AMOTION_EVENT_AXIS_LTRIGGER, AMOTION_EVENT_AXIS_RTRIGGER, AMOTION_EVENT_AXIS_BRAKE,
        AMOTION_EVENT_AXIS_GAS, AMOTION_EVENT_AXIS_HAT_X, AMOTION_EVENT_AXIS_HAT_Y}) {
    GameActivityPointerAxes_enableAxis(axis);
  }
}

void InputAdapter::process(android_input_buffer& buffer, const int surfaceWidth,
                           const int surfaceHeight) {
  for (std::uint64_t index = 0; index < buffer.keyEventsCount; ++index) {
    processKey(buffer.keyEvents[index]);
  }
  for (std::uint64_t index = 0; index < buffer.motionEventsCount; ++index) {
    processMotion(buffer.motionEvents[index], surfaceWidth, surfaceHeight);
  }
}

void InputAdapter::processKey(const GameActivityKeyEvent& event) {
  const std::optional<InputAction> action = actionForKey(event.keyCode);
  if (!action.has_value()) {
    return;
  }
  if (event.action == AKEY_EVENT_ACTION_DOWN) {
    input_.setDigital(*action, true);
  } else if (event.action == AKEY_EVENT_ACTION_UP) {
    input_.setDigital(*action, false);
  }
}

void InputAdapter::processMotion(const GameActivityMotionEvent& event, const int surfaceWidth,
                                 const int surfaceHeight) {
  if (event.pointerCount == 0) {
    return;
  }

  if ((event.source & AINPUT_SOURCE_CLASS_JOYSTICK) != 0) {
    const GameActivityPointerAxes& pointer = event.pointers[0];
    const float steering =
        withDeadZone(GameActivityPointerAxes_getAxisValue(&pointer, AMOTION_EVENT_AXIS_X));
    const float throttle =
        std::max(GameActivityPointerAxes_getAxisValue(&pointer, AMOTION_EVENT_AXIS_GAS),
                 GameActivityPointerAxes_getAxisValue(&pointer, AMOTION_EVENT_AXIS_RTRIGGER));
    const float brake =
        std::max(GameActivityPointerAxes_getAxisValue(&pointer, AMOTION_EVENT_AXIS_BRAKE),
                 GameActivityPointerAxes_getAxisValue(&pointer, AMOTION_EVENT_AXIS_LTRIGGER));
    input_.setAnalog(steering, throttle, brake);
    return;
  }

  const std::int32_t maskedAction = event.action & AMOTION_EVENT_ACTION_MASK;
  const std::uint32_t actionIndex =
      static_cast<std::uint32_t>((event.action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >>
                                 AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT);
  if (maskedAction == AMOTION_EVENT_ACTION_CANCEL) {
    input_.setPrimaryTouch(false, -1, 0.0F, 0.0F);
    return;
  }

  if ((maskedAction == AMOTION_EVENT_ACTION_DOWN ||
       maskedAction == AMOTION_EVENT_ACTION_POINTER_DOWN) &&
      actionIndex < event.pointerCount) {
    const GameActivityPointerAxes& pointer = event.pointers[actionIndex];
    input_.setPrimaryTouch(true, pointer.id, normalizedX(pointer, surfaceWidth),
                           normalizedY(pointer, surfaceHeight));
    return;
  }

  if (maskedAction == AMOTION_EVENT_ACTION_MOVE) {
    const tmnf::platform::InputSnapshot& snapshot = input_.current();
    std::uint32_t pointerIndex = 0;
    if (snapshot.touchActive) {
      for (std::uint32_t index = 0; index < event.pointerCount; ++index) {
        if (event.pointers[index].id == snapshot.touchId) {
          pointerIndex = index;
          break;
        }
      }
    }
    const GameActivityPointerAxes& pointer = event.pointers[pointerIndex];
    input_.setPrimaryTouch(true, pointer.id, normalizedX(pointer, surfaceWidth),
                           normalizedY(pointer, surfaceHeight));
    return;
  }

  if ((maskedAction == AMOTION_EVENT_ACTION_UP ||
       maskedAction == AMOTION_EVENT_ACTION_POINTER_UP) &&
      actionIndex < event.pointerCount) {
    const GameActivityPointerAxes& released = event.pointers[actionIndex];
    const tmnf::platform::InputSnapshot& snapshot = input_.current();
    if (!snapshot.touchActive || snapshot.touchId == released.id) {
      input_.setPrimaryTouch(false, -1, normalizedX(released, surfaceWidth),
                             normalizedY(released, surfaceHeight));
    }
  }
}

} // namespace tmnf::android
