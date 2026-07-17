#pragma once

#include "tmnf/gbx/input.hpp"

#include <cstddef>
#include <span>
#include <string>
#include <string_view>

namespace tmnf::gbx {

// A logical GBX input over already-decoded bytes. Initializer writes are no-ops because
// loose files and recovered archive payloads no longer have an underlying pack cipher.
class SpanInput final : public Input {
public:
  explicit SpanInput(std::span<const std::byte> bytes) : bytes_(bytes) {}

  bool readBytes(std::span<std::byte> destination) override;
  void initialize(std::span<const std::byte>) noexcept override {}

  [[nodiscard]] std::string_view error() const noexcept override { return error_; }
  [[nodiscard]] std::size_t position() const noexcept override { return position_; }
  [[nodiscard]] std::size_t size() const noexcept override { return bytes_.size(); }

private:
  std::span<const std::byte> bytes_;
  std::size_t position_ = 0;
  std::string error_;
};

} // namespace tmnf::gbx
