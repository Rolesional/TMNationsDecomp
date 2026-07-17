#include "tmnf/pak/header.hpp"

#include <array>

namespace tmnf::pak {
namespace {

constexpr std::array kMagic{
    std::byte{'N'}, std::byte{'a'}, std::byte{'d'}, std::byte{'e'},
    std::byte{'o'}, std::byte{'P'}, std::byte{'a'}, std::byte{'k'},
};

ParseResult fail(const char* const message) {
  ParseResult result;
  result.error = message;
  return result;
}

} // namespace

ParseResult parseHeader(const std::span<const std::byte> bytes) {
  if (bytes.size() < kMagic.size()) {
    return fail("truncated NadeoPak magic");
  }
  for (std::size_t index = 0; index < kMagic.size(); ++index) {
    if (bytes[index] != kMagic[index]) {
      return fail("missing NadeoPak magic");
    }
  }
  if (bytes.size() < kPreambleSize) {
    return fail("truncated NadeoPak version");
  }

  ParseResult result;
  for (std::size_t index = 0; index < sizeof(result.header.version); ++index) {
    result.header.version |=
        static_cast<std::uint32_t>(std::to_integer<unsigned char>(bytes[8 + index]))
        << (index * 8U);
  }
  if (result.header.version != kSupportedVersion) {
    return fail("unsupported NadeoPak version");
  }
  if (bytes.size() < kHeaderSize) {
    return fail("truncated NadeoPak initialization vector");
  }
  for (std::size_t index = 0; index < kInitializationVectorSize; ++index) {
    result.header.initializationVector |=
        static_cast<std::uint64_t>(std::to_integer<unsigned char>(bytes[12 + index]))
        << (index * 8U);
  }
  result.header.encryptedHeaderOffset = kHeaderSize;
  return result;
}

} // namespace tmnf::pak
