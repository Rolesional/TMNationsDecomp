#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string_view>

namespace tmnf::gbx {

struct ClassInfo final {
  std::uint32_t id;
  std::uint32_t parentId;
  std::string_view name;
};

[[nodiscard]] std::span<const ClassInfo> classRegistry() noexcept;
[[nodiscard]] const ClassInfo* findClass(std::uint32_t id) noexcept;
[[nodiscard]] std::optional<std::string_view> className(std::uint32_t id) noexcept;
[[nodiscard]] std::optional<std::uint32_t> classParentId(std::uint32_t id) noexcept;

// Returns the four-byte class seed passed to CClassicBufferCrypted when the
// node body starts. This normally is the direct parent class ID, with the
// compatibility substitutions made by TmForever.exe.
[[nodiscard]] std::optional<std::uint32_t> dummyWriteSeedClassId(std::uint32_t id) noexcept;

} // namespace tmnf::gbx
