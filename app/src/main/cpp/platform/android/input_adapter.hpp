#pragma once

#include "tmnf/platform/input.hpp"

struct android_input_buffer;
struct GameActivityKeyEvent;
struct GameActivityMotionEvent;

namespace tmnf::android {

class InputAdapter final {
public:
  explicit InputAdapter(tmnf::platform::InputState& input);

  void process(android_input_buffer& buffer, int surfaceWidth, int surfaceHeight);

private:
  void processKey(const GameActivityKeyEvent& event);
  void processMotion(const GameActivityMotionEvent& event, int surfaceWidth, int surfaceHeight);

  tmnf::platform::InputState& input_;
};

} // namespace tmnf::android
