#include "tmnf/crypto/md5.hpp"
#include "tmnf/pak/pack_list.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <span>
#include <string>
#include <vector>

namespace {

void appendU32(std::vector<std::byte>& bytes, const std::uint32_t value) {
  for (std::size_t index = 0; index < 4; ++index) {
    bytes.push_back(static_cast<std::byte>((value >> (index * 8U)) & 0xffU));
  }
}

} // namespace

int main() {
  if (tmnf::crypto::toHex(tmnf::crypto::md5(""), false) != "d41d8cd98f00b204e9800998ecf8427e" ||
      tmnf::crypto::toHex(tmnf::crypto::md5("abc"), false) != "900150983cd24fb0d6963f7d28e17f72") {
    std::cerr << "MD5 did not match published test vectors\n";
    return 1;
  }

  constexpr std::uint32_t salt = 123;
  constexpr std::string_view name = "sample";
  constexpr std::string_view keyText = "00112233445566778899AABBCCDDEEFF";
  const tmnf::crypto::Md5Digest nameKey = tmnf::crypto::md5("6611992868945B0B59536FC3226F3FD0123");
  const tmnf::crypto::Md5Digest keyStringKey =
      tmnf::crypto::md5("sample123B97C1205648A66E04F86A1B5D5AF9862");

  std::vector<std::byte> fixture;
  fixture.push_back(std::byte{1});
  fixture.push_back(std::byte{1});
  appendU32(fixture, 0x12345678);
  appendU32(fixture, salt);
  fixture.push_back(std::byte{5});
  fixture.push_back(static_cast<std::byte>(name.size()));
  for (std::size_t index = 0; index < name.size(); ++index) {
    fixture.push_back(static_cast<std::byte>(name[index]) ^ nameKey[index % 16U]);
  }
  for (std::size_t index = 0; index < keyText.size(); ++index) {
    fixture.push_back(static_cast<std::byte>(keyText[index]) ^ keyStringKey[index % 16U]);
  }
  for (std::size_t index = 0; index < 16; ++index) {
    fixture.push_back(static_cast<std::byte>(index));
  }

  const tmnf::pak::PackListParseResult parsed = tmnf::pak::parsePackList(fixture);
  if (!parsed || parsed.packList.entries.size() != 1 ||
      parsed.packList.entries.front().name != name ||
      tmnf::crypto::toHex(parsed.packList.entries.front().baseKey) != keyText) {
    std::cerr << "synthetic packlist fixture did not parse\n";
    return 1;
  }
  return 0;
}
