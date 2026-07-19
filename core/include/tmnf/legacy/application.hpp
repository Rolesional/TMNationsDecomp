#pragma once

#include "tmnf/legacy/string_view.hpp"

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <utility>

namespace tmnf::legacy {

// CGbxGame vslot +0xA0, FUN_00401450.
template <typename Handler, typename Argument>
[[nodiscard]] bool handleGameArgument(Handler* const handler, Argument argument) {
  return handler != nullptr && handler->handleGameArgument(argument);
}

// CGbxGame::StopApp(), vslot +0x88, FUN_00401490.
template <typename Handler, typename Fallback>
void stopGameApplication(Handler* const handler, Fallback fallback) {
  if (handler != nullptr) {
    handler->stopApplication();
  } else {
    fallback();
  }
}

// CGbxGame vslot +0xA4, FUN_00401A00.
[[nodiscard]] inline bool setPendingGameArgument(
    std::u16string* const pending, const std::u16string_view argument) {
  if (pending == nullptr || !pending->empty()) {
    return false;
  }
  assignView(*pending, argument);
  return true;
}

// CGbxGame::StopApp cleanup body, FUN_00401A50.
template <typename Application, typename TraceStop, typename StopGlobalTask>
void stopGbxGame(
    Application*& application,
    std::uint32_t& diagnosticLength,
    char* const diagnosticData,
    std::uint32_t& preservedState,
    TraceStop traceStop,
    StopGlobalTask stopGlobalTask) {
  clearBuffer(diagnosticLength, diagnosticData);
  traceStop();
  stopGlobalTask();
  if (application == nullptr) {
    return;
  }
  if (application->hasExtension()) {
    application->destroyExtension();
  }
  preservedState = application->stopState();
  application->addRef();
  application->stopApplication();
  application->release();
  application = nullptr;
}

// Unnamed CGbxGame vslot +0x94, FUN_00401B20.
[[nodiscard]] constexpr bool gbxGameSlot94Predicate(
    const std::uint32_t field3c,
    const std::uint32_t field100,
    const std::uint32_t field104,
    const std::uint32_t field10c,
    const std::uint32_t field11c) noexcept {
  if ((field100 == 0U || field104 != 0U || field10c != 0U) && field11c == 0U) {
    return field3c != 0U;
  }
  return true;
}

// CFastCallbackInstance1P<CGbxApp, ...>::vftable slot 0,
// FUN_00401B80 and FUN_00401BB0.
template <typename Owner, typename Callback, typename Argument>
decltype(auto) invokeFastCallbackInstance1P(
    Owner* const owner, Callback callback, Argument&& argument) {
  return std::invoke(callback, owner, std::forward<Argument>(argument));
}

} // namespace tmnf::legacy
