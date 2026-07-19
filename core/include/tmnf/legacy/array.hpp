#pragma once

#include <cstdint>

namespace tmnf::legacy {

template <typename Element>
struct ArrayStorage {
  std::uint32_t count{};
  Element* data{};
};

// FUN_00401770 (32-bit elements) and FUN_004560F0 (byte elements).
template <typename Element>
[[nodiscard]] Element* arrayElement(
    const ArrayStorage<Element>& storage, const std::uint32_t index) noexcept {
  return storage.data + index;
}

} // namespace tmnf::legacy
