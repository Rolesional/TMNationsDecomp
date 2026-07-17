#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace tmnf::platform {

enum class InputAction : std::uint8_t {
  Accelerate,
  Brake,
  SteerLeft,
  SteerRight,
  Confirm,
  Cancel,
  Respawn,
  Menu,
  Count,
};

inline constexpr std::size_t kInputActionCount = static_cast<std::size_t>(InputAction::Count);

struct InputSnapshot final {
  std::array<bool, kInputActionCount> held{};
  std::array<bool, kInputActionCount> pressed{};
  std::array<bool, kInputActionCount> released{};
  float steering = 0.0F;
  float throttle = 0.0F;
  float brake = 0.0F;
  float touchX = 0.0F;
  float touchY = 0.0F;
  std::int32_t touchId = -1;
  bool touchActive = false;
  bool focused = false;

  [[nodiscard]] bool isHeld(InputAction action) const noexcept;
  [[nodiscard]] bool wasPressed(InputAction action) const noexcept;
  [[nodiscard]] bool wasReleased(InputAction action) const noexcept;
};

// Platform-neutral input accumulator. Digital transitions survive until the
// simulation consumes a snapshot, while held state and analog values persist.
class InputState final {
public:
  void setDigital(InputAction action, bool down) noexcept;
  void setAnalog(float steering, float throttle, float brake) noexcept;
  void setPrimaryTouch(bool active, std::int32_t id, float x, float y) noexcept;
  void setFocused(bool focused) noexcept;
  void releaseAll() noexcept;

  [[nodiscard]] const InputSnapshot& current() const noexcept;
  [[nodiscard]] InputSnapshot consume() noexcept;

private:
  InputSnapshot state_;
};

} // namespace tmnf::platform
