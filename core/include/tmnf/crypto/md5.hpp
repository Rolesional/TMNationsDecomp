#pragma once

#include <array>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>

namespace tmnf::crypto {

using Md5Digest = std::array<std::byte, 16>;

[[nodiscard]] Md5Digest md5(std::span<const std::byte> bytes);
[[nodiscard]] Md5Digest md5(std::string_view text);
[[nodiscard]] std::string toHex(const Md5Digest& digest, bool uppercase = true);

} // namespace tmnf::crypto
