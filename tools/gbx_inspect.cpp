#include "tmnf/gbx/class_registry.hpp"
#include "tmnf/gbx/document.hpp"
#include "tmnf/gbx/header.hpp"
#include "tmnf/gbx/span_input.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace {

std::string jsonChar(const char value) {
  if (value >= 0x20 && value <= 0x7E && value != '"' && value != '\\') {
    return std::string(1, value);
  }
  return "?";
}

std::string jsonEscape(const std::string_view value) {
  std::string result;
  result.reserve(value.size());
  for (const unsigned char character : value) {
    if (character == '"' || character == '\\') {
      result.push_back('\\');
      result.push_back(static_cast<char>(character));
    } else if (character >= 0x20U) {
      result.push_back(static_cast<char>(character));
    }
  }
  return result;
}

void printOptionalHex(const std::optional<std::uint32_t> value) {
  if (!value.has_value()) {
    std::cout << "null";
    return;
  }
  std::cout << "\"0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << *value
            << std::dec << "\"";
}

} // namespace

int main(const int argc, const char* const* argv) {
  if (argc != 2) {
    std::cerr << "usage: gbx_inspect <file.gbx>\n";
    return 2;
  }

  const std::filesystem::path path(argv[1]);
  std::ifstream stream(path, std::ios::binary);
  if (!stream) {
    std::cerr << "unable to open: " << path << '\n';
    return 2;
  }

  const std::vector<char> raw((std::istreambuf_iterator<char>(stream)),
                              std::istreambuf_iterator<char>());
  const auto bytes = std::span(reinterpret_cast<const std::byte*>(raw.data()), raw.size());
  const tmnf::gbx::ParseResult parsed = tmnf::gbx::parseHeader(bytes);
  if (!parsed) {
    std::cerr << "invalid GBX: " << parsed.error << '\n';
    return 1;
  }

  tmnf::gbx::SpanInput input(bytes);
  const tmnf::gbx::DocumentProbeResult probed = tmnf::gbx::probeDocument(input, false);
  if (!probed) {
    std::cerr << "invalid GBX document: " << probed.error << '\n';
    return 1;
  }

  const tmnf::gbx::Header& header = parsed.header;
  const tmnf::gbx::DocumentProbe& document = probed.document;
  std::cout << "{\n"
            << "  \"path\": \"" << path.generic_string() << "\",\n"
            << "  \"version\": " << header.version << ",\n"
            << "  \"format\": \"" << jsonChar(header.format) << "\",\n"
            << "  \"referenceTableCompression\": \"" << jsonChar(header.referenceTableCompression)
            << "\",\n"
            << "  \"bodyCompression\": \"" << jsonChar(header.bodyCompression) << "\",\n"
            << "  \"classId\": \"0x" << std::hex << std::uppercase << std::setw(8)
            << std::setfill('0') << header.classId << std::dec << "\",\n"
            << "  \"className\": \"" << tmnf::gbx::className(header.classId).value_or("<unknown>")
            << "\",\n"
            << "  \"userDataSize\": " << header.userDataSize << ",\n"
            << "  \"bodyOffset\": " << header.bodyOffset << ",\n"
            << "  \"userDataChunks\": [\n";

  for (std::size_t index = 0; index < header.userDataChunks.size(); ++index) {
    const tmnf::gbx::UserDataChunk& chunk = header.userDataChunks[index];
    std::cout << "    {\"id\": \"0x" << std::hex << std::uppercase << std::setw(8)
              << std::setfill('0') << chunk.id << std::dec << "\", \"size\": " << chunk.size
              << ", \"heavy\": " << (chunk.heavy ? "true" : "false") << "}"
              << (index + 1 == header.userDataChunks.size() ? "" : ",") << '\n';
  }

  std::cout << "  ],\n"
            << "  \"nodeCount\": " << document.numNodes << ",\n"
            << "  \"ancestorLevel\": " << document.ancestorLevel << ",\n"
            << "  \"directoryCount\": " << document.directories.size() << ",\n"
            << "  \"externalNodes\": [\n";
  for (std::size_t index = 0; index < document.externalNodes.size(); ++index) {
    const tmnf::gbx::ExternalNode& node = document.externalNodes[index];
    std::cout << "    {\"flags\": " << node.flags << ", \"nodeIndex\": " << node.nodeIndex
              << ", \"useFile\": " << (node.useFile ? "true" : "false") << ", \"path\": \""
              << jsonEscape(tmnf::gbx::externalNodePath(document, node))
              << "\", \"resourceIndex\": ";
    if (node.resourceIndex.has_value()) {
      std::cout << *node.resourceIndex;
    } else {
      std::cout << "null";
    }
    std::cout << "}" << (index + 1U == document.externalNodes.size() ? "" : ",") << '\n';
  }
  std::cout << "  ],\n"
            << "  \"bodyHeaderOffset\": " << document.bodyHeaderOffset << ",\n"
            << "  \"bodyDataOffset\": " << document.bodyDataOffset << ",\n"
            << "  \"bodyUncompressedSize\": ";
  if (document.bodyUncompressedSize.has_value()) {
    std::cout << *document.bodyUncompressedSize;
  } else {
    std::cout << "null";
  }
  std::cout << ",\n  \"bodyCompressedSize\": ";
  if (document.bodyCompressedSize.has_value()) {
    std::cout << *document.bodyCompressedSize;
  } else {
    std::cout << "null";
  }
  std::cout << ",\n  \"dummyWriteSeed\": ";
  printOptionalHex(document.dummyWriteSeed);
  std::cout << ",\n  \"firstChunkId\": ";
  printOptionalHex(document.firstChunkId);
  std::cout << "\n}\n";
  return 0;
}
