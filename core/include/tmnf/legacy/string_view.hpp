#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace tmnf::legacy {

// FUN_00401000 (char) and FUN_004010A0 (UTF-16).
template <typename Character>
[[nodiscard]] std::basic_string_view<Character>
nullableStringView(const Character* const value) noexcept {
  return value == nullptr ? std::basic_string_view<Character>{}
                          : std::basic_string_view<Character>{value};
}

// FUN_00452C20 (char) and FUN_004598B0 (UTF-16) preserve a view already backed by the
// destination buffer.
template <typename Character>
inline std::basic_string<Character>& assignView(
    std::basic_string<Character>& destination,
    const std::basic_string_view<Character> value) {
  if (destination.data() == value.data()) {
    return destination;
  }
  if (value.empty()) {
    destination.clear();
  } else {
    destination.assign(value.data(), value.size());
  }
  return destination;
}

// FUN_00401030.
inline std::string& assignNullable(std::string& destination, const char* const value) {
  return assignView(destination, nullableStringView(value));
}

template <typename Character>
struct StringStorageView {
  std::uint32_t length{};
  const Character* data{};
};

// FUN_00401810 adapts the engine's length-first UTF-16 view to its owned string.
template <typename Character>
inline std::basic_string<Character>& assignStorage(
    std::basic_string<Character>& destination,
    const StringStorageView<Character>& source) {
  return assignView(destination, {source.data, source.length});
}

// FUN_00401650 clears a non-empty fixed buffer without reallocating it.
inline void clearBuffer(std::uint32_t& length, char* const data) noexcept {
  if (length != 0U) {
    length = 0U;
    data[0] = '\0';
  }
}

// FUN_00401C40 (narrow) and FUN_00401D30 (UTF-16).
template <typename Character>
void initializeStringStorage(
    std::uint32_t& length, Character*& data, Character* const emptyData) noexcept {
  length = 0U;
  data = emptyData;
}

// Narrow: FUN_004016F0, FUN_00401BC0, and FUN_00401C60.
// UTF-16: FUN_00401730, FUN_00401C00, and FUN_00401DB0.
template <typename Character, typename Deallocate>
void releaseStringStorage(
    std::uint32_t& length,
    Character*& data,
    Character* const emptyData,
    Deallocate deallocate) {
  if (data == emptyData) {
    return;
  }
  auto* const bytes = reinterpret_cast<unsigned char*>(data);
  auto* const allocation = bytes - ((bytes[-1] & 0x80U) != 0U ? 4U : sizeof(Character));
  deallocate(allocation);
  initializeStringStorage(length, data, emptyData);
}

// Construct a narrow fast string from a nullable C string, FUN_00401CA0.
[[nodiscard]] inline std::string makeNullableString(const char* const value) {
  return std::string{nullableStringView(value)};
}

// The engine's permissive null-terminated UTF-8 decoder, FUN_00459050.
[[nodiscard]] inline std::uint32_t decodeLegacyUtf8CodePoint(
    unsigned char*& cursor) noexcept {
  const auto isContinuation = [](const std::uint32_t value) {
    return (value ^ 0x80U) < 0x40U;
  };

  for (;;) {
    auto* const bytes = cursor;
    const std::uint32_t lead = bytes[0];
    if (lead == 0U) {
      ++cursor;
      return 0U;
    }
    if (lead < 0x80U) {
      ++cursor;
      return lead;
    }
    if (lead < 0xc2U) {
      ++cursor;
      continue;
    }
    if (lead < 0xe0U) {
      const std::uint32_t second = bytes[1];
      const bool preserveMalformedSecond =
          second == 0x0aU || second == 0x09U ||
          (second >= 0x20U && second <= 0x2fU) ||
          (second >= 0x3aU && second <= 0x3fU) ||
          (second >= 0x7bU && second <= 0x7dU);
      if (!isContinuation(second) && !preserveMalformedSecond) {
        bytes[1] = static_cast<unsigned char>((second & 0x3fU) | 0x80U);
      }
      if (isContinuation(second)) {
        cursor += 2;
        return ((lead & 0x1fU) << 6U) | (second ^ 0x80U);
      }
      ++cursor;
      continue;
    }

    const auto combine = [bytes](std::uint32_t value, const std::size_t count) {
      for (std::size_t index = 1; index < count; ++index) {
        value = (value << 6U) | (bytes[index] ^ 0x80U);
      }
      return value;
    };
    if (lead < 0xf0U) {
      if (!isContinuation(bytes[1]) || !isContinuation(bytes[2]) ||
          (lead == 0xe0U && bytes[1] < 0xa0U)) {
        ++cursor;
        continue;
      }
      cursor += 3;
      const std::uint32_t value = combine(lead & 0x0fU, 3U);
      if (value == 0xfeffU) {
        continue;
      }
      return value;
    }
    if (lead < 0xf8U) {
      if (!isContinuation(bytes[1]) || !isContinuation(bytes[2]) ||
          !isContinuation(bytes[3]) ||
          (lead == 0xf0U && bytes[1] < 0x90U)) {
        ++cursor;
        continue;
      }
      cursor += 4;
      return combine(lead & 0x07U, 4U);
    }
    if (lead < 0xfcU) {
      if (!isContinuation(bytes[1]) || !isContinuation(bytes[2]) ||
          !isContinuation(bytes[3]) || !isContinuation(bytes[4]) ||
          (lead == 0xf8U && bytes[1] < 0x88U)) {
        ++cursor;
        continue;
      }
      cursor += 5;
      return combine(lead & 0x03U, 5U);
    }
    if (lead < 0xfeU) {
      if (!isContinuation(bytes[1]) || !isContinuation(bytes[2]) ||
          !isContinuation(bytes[3]) || !isContinuation(bytes[4]) ||
          !isContinuation(bytes[5]) ||
          (lead == 0xfcU && bytes[1] < 0x84U)) {
        ++cursor;
        continue;
      }
      cursor += 6;
      return combine(lead & 0x01U, 6U);
    }
    ++cursor;
  }
}

// Decode a BOM-marked narrow fast string into UTF-16 storage, FUN_0045A660.
inline void assignUtf8BomWide(
    std::u16string& destination, const StringStorageView<char>& source) {
  destination.clear();
  destination.reserve(source.length);
  auto* cursor = const_cast<unsigned char*>(
      reinterpret_cast<const unsigned char*>(source.data));
  for (std::uint32_t value = decodeLegacyUtf8CodePoint(cursor); value != 0U;
       value = decodeLegacyUtf8CodePoint(cursor)) {
    if (value != 0xfeffU) {
      destination.push_back(static_cast<char16_t>(value));
    }
  }
}

// Convert a narrow fast string to UTF-16, FUN_0045A9D0.
inline void assignNarrowAsWide(
    std::u16string& destination, const StringStorageView<char>& source) {
  const auto* const bytes = reinterpret_cast<const unsigned char*>(source.data);
  if (bytes != nullptr && bytes[0] == 0xefU && bytes[1] == 0xbbU &&
      bytes[2] == 0xbfU) {
    assignUtf8BomWide(destination, source);
    return;
  }
  destination.resize(source.length);
  for (std::uint32_t index = 0; index < source.length; ++index) {
    destination[index] = static_cast<char16_t>(bytes[index]);
  }
}

// Construct a UTF-16 fast string from narrow storage, FUN_00401D50.
[[nodiscard]] inline std::u16string makeWideFastString(
    const StringStorageView<char>& source) {
  std::u16string result;
  assignNarrowAsWide(result, source);
  return result;
}

// Construct a UTF-16 fast string from a nullable pointer, FUN_00401DF0.
[[nodiscard]] inline std::u16string makeNullableWideString(const char16_t* const value) {
  return std::u16string{nullableStringView(value)};
}

} // namespace tmnf::legacy
