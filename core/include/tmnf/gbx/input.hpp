#pragma once

#include <cstddef>
#include <span>
#include <string_view>

namespace tmnf::gbx {

class Input {
public:
  virtual ~Input() = default;

  virtual bool readBytes(std::span<std::byte> destination) = 0;
  virtual void initialize(std::span<const std::byte> bytes) noexcept = 0;

  [[nodiscard]] virtual std::string_view error() const noexcept = 0;
  [[nodiscard]] virtual std::size_t position() const noexcept = 0;
  [[nodiscard]] virtual std::size_t size() const noexcept = 0;
};

} // namespace tmnf::gbx
