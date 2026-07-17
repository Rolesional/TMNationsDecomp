#include "tmnf/gbx/class_registry.hpp"

#include <algorithm>
#include <array>

namespace tmnf::gbx {
namespace {

constexpr std::array kClassRegistry{
#include "class_registry.inc"
};

consteval bool registryIsStrictlySorted() {
  for (std::size_t index = 1; index < kClassRegistry.size(); ++index) {
    if (kClassRegistry[index - 1].id >= kClassRegistry[index].id) {
      return false;
    }
  }
  return true;
}

static_assert(registryIsStrictlySorted());

} // namespace

std::span<const ClassInfo> classRegistry() noexcept { return kClassRegistry; }

const ClassInfo* findClass(const std::uint32_t id) noexcept {
  const auto found = std::lower_bound(
      kClassRegistry.begin(), kClassRegistry.end(), id,
      [](const ClassInfo& info, const std::uint32_t candidate) { return info.id < candidate; });
  if (found == kClassRegistry.end() || found->id != id) {
    return nullptr;
  }
  return &*found;
}

std::optional<std::string_view> className(const std::uint32_t id) noexcept {
  const ClassInfo* const info = findClass(id);
  return info == nullptr ? std::nullopt : std::optional(info->name);
}

std::optional<std::uint32_t> classParentId(const std::uint32_t id) noexcept {
  const ClassInfo* const info = findClass(id);
  return info == nullptr ? std::nullopt : std::optional(info->parentId);
}

std::optional<std::uint32_t> dummyWriteSeedClassId(const std::uint32_t id) noexcept {
  const ClassInfo* const info = findClass(id);
  if (info == nullptr || info->parentId == 0) {
    return std::nullopt;
  }

  switch (info->parentId) {
  case 0x07031000U: // CControlText
    return 0x07001000U;
  case 0x0304E000U: // CGameCtnBlockInfo
    return 0x24005000U;
  case 0x090EB000U: // CPlugVehiclePhyTuning
    return 0x0A02E000U;
  case 0x0501A000U: // CFuncKeysReal
    return 0x01001000U;
  case 0x0305C000U: // CGameCtnZone
    return 0x2401C000U;
  default:
    return info->parentId;
  }
}

} // namespace tmnf::gbx
