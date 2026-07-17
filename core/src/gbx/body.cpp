#include "tmnf/gbx/body.hpp"

#include "tmnf/compression/lzo.hpp"

#include <algorithm>
#include <utility>

namespace tmnf::gbx {
namespace {

BodyDecodeResult failed(std::string message) {
  BodyDecodeResult result;
  result.error = std::move(message);
  return result;
}

} // namespace

BodyDecodeResult decodeBody(const std::span<const std::byte> documentBytes,
                            const DocumentProbe& document) {
  if (document.bodyDataOffset > documentBytes.size()) {
    return failed("GBX body offset exceeds the document");
  }
  if (document.header.bodyCompression == 'U') {
    BodyDecodeResult result;
    result.data.assign(documentBytes.begin() + static_cast<std::ptrdiff_t>(document.bodyDataOffset),
                       documentBytes.end());
    return result;
  }
  if (document.header.bodyCompression != 'C' || !document.bodyCompressedSize.has_value() ||
      !document.bodyUncompressedSize.has_value()) {
    return failed("GBX body compression metadata is incomplete");
  }
  const std::size_t compressedSize = *document.bodyCompressedSize;
  if (compressedSize > documentBytes.size() - document.bodyDataOffset) {
    return failed("compressed GBX body exceeds the document");
  }
  const std::span compressed = documentBytes.subspan(document.bodyDataOffset, compressedSize);
  compression::LzoDecompressResult decompressed =
      compression::decompressLzo1x(compressed, *document.bodyUncompressedSize);
  if (!decompressed) {
    return failed(decompressed.error);
  }
  BodyDecodeResult result;
  result.data = std::move(decompressed.data);
  return result;
}

} // namespace tmnf::gbx
