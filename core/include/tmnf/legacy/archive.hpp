#pragma once

#include <cstdint>

namespace tmnf::legacy {

inline constexpr std::uint32_t kArchiveNodeWriteMode = 7U;

// FUN_00401110: scoped CSystemArchiveNod write wrapper.
template <typename Archive, typename Node>
[[nodiscard]] decltype(auto) writeArchiveNode(
    Node* const node, const std::uint32_t flags, const bool transactional) {
  Archive archive;
  return archive.writeNode(node, flags, kArchiveNodeWriteMode, transactional);
}

// FUN_004011A0: scoped CSystemArchiveNod read wrapper.
template <typename Archive, typename First, typename Second, typename Third, typename Fourth>
[[nodiscard]] decltype(auto) readArchiveNode(
    First first, Second second, Third third, Fourth fourth) {
  Archive archive;
  return archive.readNode(first, second, third, fourth);
}

} // namespace tmnf::legacy
