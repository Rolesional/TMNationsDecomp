#include "tmnf/gbx/span_input.hpp"

#include <algorithm>

namespace tmnf::gbx {

bool SpanInput::readBytes(const std::span<std::byte> destination) {
  if (!error_.empty()) {
    return false;
  }
  if (position_ > bytes_.size() || destination.size() > bytes_.size() - position_) {
    error_ = "read exceeds the GBX byte span";
    return false;
  }
  std::copy_n(bytes_.begin() + static_cast<std::ptrdiff_t>(position_), destination.size(),
              destination.begin());
  position_ += destination.size();
  return true;
}

} // namespace tmnf::gbx
