#pragma once

#include <cstdint>
#include <span>
#include <string_view>

namespace tmnf::legacy {

inline constexpr std::uint32_t kTranslationNotFound = 0xffffffffU;

struct TranslationCatalog {
  std::span<const char16_t* const> sourceStrings{};
  std::span<const char16_t* const> translatedStrings{};
  std::span<std::uint8_t> used{};
  std::uint32_t usedCount{};
  std::span<const std::uint16_t> hashSlots{};
};

// FUN_00465680: GNU gettext's UTF-16 string hash.
[[nodiscard]] inline std::uint32_t gettextHash(
    const char16_t* value, std::uint32_t* const length = nullptr) noexcept {
  std::uint32_t hash = 0;
  std::uint32_t count = 0;
  while (*value != u'\0') {
    hash = (hash << 4U) + static_cast<std::uint32_t>(*value++);
    ++count;
    if (const auto high = hash & 0xf0000000U; high != 0U) {
      hash ^= high ^ (high >> 24U);
    }
  }
  if (length != nullptr) {
    *length = count;
  }
  return hash;
}

// FUN_00455E20: hide a gettext context encoded as |context|text.
[[nodiscard]] inline const char16_t* stripTranslationContext(
    const char16_t* const value) noexcept {
  if (*value != u'|') {
    return value;
  }
  for (auto* cursor = value + 1; *cursor != u'\0'; ++cursor) {
    if (*cursor == u'|') {
      return cursor + 1;
    }
  }
  return value;
}

// FUN_00456200: open-addressed catalog lookup with first-use accounting.
[[nodiscard]] inline std::uint32_t findTranslation(
    TranslationCatalog& catalog, const char16_t* const value) noexcept {
  const auto slotCount = static_cast<std::uint32_t>(catalog.hashSlots.size());
  const auto hash = gettextHash(value);
  auto slot = hash % slotCount;
  const auto step = hash % (slotCount - 2U) + 1U;
  for (;;) {
    const auto index = catalog.hashSlots[slot];
    if (index == 0xffffU) {
      return kTranslationNotFound;
    }
    if (std::u16string_view{catalog.sourceStrings[index]} ==
        std::u16string_view{value}) {
      if (catalog.used[index] == 0U) {
        catalog.used[index] = 1U;
        ++catalog.usedCount;
      }
      return index;
    }
    slot = (slot + step) % slotCount;
  }
}

// FUN_004565F0.
[[nodiscard]] inline const char16_t* resolveTranslation(
    TranslationCatalog& catalog, const char16_t* value) noexcept {
  if (*value == u'\0') {
    return value;
  }
  if (!catalog.sourceStrings.empty() && !catalog.hashSlots.empty()) {
    const auto index = findTranslation(catalog, value);
    if (index != kTranslationNotFound) {
      value = catalog.translatedStrings[index];
    }
  }
  return stripTranslationContext(value);
}

[[nodiscard]] inline TranslationCatalog& translationCatalog() noexcept {
  static TranslationCatalog catalog;
  return catalog;
}

// FUN_004010F0: wrapper around the process-global translation catalog.
[[nodiscard]] inline const char16_t* translate(const char16_t* const value) noexcept {
  return resolveTranslation(translationCatalog(), value);
}

} // namespace tmnf::legacy
