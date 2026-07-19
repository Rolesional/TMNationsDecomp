#pragma once

#include <array>

namespace tmnf::legacy {

struct OriginVectorPair {
  std::array<float, 3> origin{};
  std::array<float, 3> vector{};
};

// FUN_00401860.
inline void initializeOriginVector(
    OriginVectorPair& destination, const std::array<float, 3>& vector) noexcept {
  destination.origin = {};
  destination.vector = vector;
}

} // namespace tmnf::legacy
