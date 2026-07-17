#pragma once

#include "tmnf/gbx/document.hpp"

#include <cstddef>
#include <span>
#include <string>
#include <vector>

namespace tmnf::gbx {

struct BodyDecodeResult final {
  std::vector<std::byte> data;
  std::string error;

  [[nodiscard]] explicit operator bool() const noexcept { return error.empty(); }
};

// Returns the logical uncompressed body bytes described by a previously probed
// loose/recovered GBX document.
[[nodiscard]] BodyDecodeResult decodeBody(std::span<const std::byte> documentBytes,
                                          const DocumentProbe& document);

} // namespace tmnf::gbx
