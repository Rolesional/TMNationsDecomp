#include "tmnf/pak/pack_list.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <string_view>

namespace tmnf::pak {
namespace {

constexpr std::string_view kNameKeySalt = "6611992868945B0B59536FC3226F3FD0";
constexpr std::string_view kKeyStringSalt = "B97C1205648A66E04F86A1B5D5AF9862";
constexpr std::string_view kArchiveKeySuffix = "NadeoPak";

class Reader final {
public:
  explicit Reader(const std::span<const std::byte> bytes) : bytes_(bytes) {}

  bool readU8(std::uint8_t& value) {
    if (!has(1)) {
      return false;
    }
    value = std::to_integer<std::uint8_t>(bytes_[position_++]);
    return true;
  }

  bool readU32(std::uint32_t& value) {
    if (!has(4)) {
      return false;
    }
    value = 0;
    for (std::size_t index = 0; index < 4; ++index) {
      value |= static_cast<std::uint32_t>(std::to_integer<unsigned char>(bytes_[position_ + index]))
               << (index * 8U);
    }
    position_ += 4;
    return true;
  }

  bool readBytes(const std::size_t count, std::span<const std::byte>& value) {
    if (!has(count)) {
      return false;
    }
    value = bytes_.subspan(position_, count);
    position_ += count;
    return true;
  }

  [[nodiscard]] std::size_t remaining() const noexcept { return bytes_.size() - position_; }

private:
  [[nodiscard]] bool has(const std::size_t count) const noexcept {
    return count <= bytes_.size() - position_;
  }

  std::span<const std::byte> bytes_;
  std::size_t position_ = 0;
};

PackListParseResult fail(const char* const message) {
  PackListParseResult result;
  result.error = message;
  return result;
}

std::string saltText(const std::uint32_t salt) {
  std::array<char, 10> buffer{};
  const auto [end, error] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), salt);
  if (error != std::errc{}) {
    return {};
  }
  return std::string(buffer.data(), end);
}

int hexValue(const char value) {
  if (value >= '0' && value <= '9') {
    return value - '0';
  }
  if (value >= 'a' && value <= 'f') {
    return value - 'a' + 10;
  }
  if (value >= 'A' && value <= 'F') {
    return value - 'A' + 10;
  }
  return -1;
}

bool decodeKey(const std::string_view encoded, crypto::Md5Digest& key) {
  if (encoded.size() != key.size() * 2U) {
    return false;
  }
  for (std::size_t index = 0; index < key.size(); ++index) {
    const int high = hexValue(encoded[index * 2U]);
    const int low = hexValue(encoded[index * 2U + 1U]);
    if (high < 0 || low < 0) {
      return false;
    }
    key[index] = static_cast<std::byte>((high << 4U) | low);
  }
  return true;
}

} // namespace

PackListParseResult parsePackList(const std::span<const std::byte> bytes) {
  Reader reader(bytes);
  PackListParseResult result;
  PackList& packList = result.packList;
  std::uint8_t entryCount = 0;
  if (!reader.readU8(packList.version) || !reader.readU8(entryCount) ||
      !reader.readU32(packList.crc32) || !reader.readU32(packList.salt)) {
    return fail("truncated packlist preamble");
  }
  if (packList.version != 1) {
    return fail("unsupported packlist version");
  }

  const std::string salt = saltText(packList.salt);
  const crypto::Md5Digest nameKey = crypto::md5(std::string(kNameKeySalt) + salt);
  packList.entries.reserve(entryCount);
  for (std::uint16_t index = 0; index < entryCount; ++index) {
    PackListEntry entry;
    std::uint8_t nameLength = 0;
    std::span<const std::byte> encryptedName;
    std::span<const std::byte> encryptedKey;
    if (!reader.readU8(entry.flags) || !reader.readU8(nameLength) ||
        !reader.readBytes(nameLength, encryptedName) || !reader.readBytes(32, encryptedKey)) {
      return fail("truncated packlist entry");
    }

    entry.name.resize(nameLength);
    for (std::size_t byteIndex = 0; byteIndex < encryptedName.size(); ++byteIndex) {
      entry.name[byteIndex] = static_cast<char>(
          std::to_integer<unsigned char>(encryptedName[byteIndex] ^ nameKey[byteIndex % 16U]));
    }

    const crypto::Md5Digest keyStringKey =
        crypto::md5(entry.name + salt + std::string(kKeyStringSalt));
    std::array<char, 32> keyText{};
    for (std::size_t byteIndex = 0; byteIndex < encryptedKey.size(); ++byteIndex) {
      keyText[byteIndex] = static_cast<char>(
          std::to_integer<unsigned char>(encryptedKey[byteIndex] ^ keyStringKey[byteIndex % 16U]));
    }
    if (!decodeKey(std::string_view(keyText.data(), keyText.size()), entry.baseKey)) {
      return fail("packlist entry contains an invalid key");
    }
    packList.entries.push_back(std::move(entry));
  }

  std::span<const std::byte> signature;
  if (!reader.readBytes(packList.signature.size(), signature)) {
    return fail("missing packlist signature");
  }
  std::copy(signature.begin(), signature.end(), packList.signature.begin());
  if (reader.remaining() != 0) {
    return fail("unexpected data after packlist signature");
  }
  return result;
}

crypto::Md5Digest deriveArchiveKey(const crypto::Md5Digest& baseKey) {
  return crypto::md5(crypto::toHex(baseKey) + std::string(kArchiveKeySuffix));
}

} // namespace tmnf::pak
