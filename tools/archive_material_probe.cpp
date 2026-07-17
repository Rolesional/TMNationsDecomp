#include "tmnf/content/materials.hpp"
#include "tmnf/crypto/md5.hpp"
#include "tmnf/gbx/solid.hpp"
#include "tmnf/pak/vfs.hpp"

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

bool readFile(const std::filesystem::path& path, std::vector<std::byte>& output) {
  std::ifstream stream(path, std::ios::binary);
  if (!stream) {
    return false;
  }
  stream.seekg(0, std::ios::end);
  const std::streamoff size = stream.tellg();
  if (size < 0) {
    return false;
  }
  output.resize(static_cast<std::size_t>(size));
  stream.seekg(0, std::ios::beg);
  if (!output.empty()) {
    stream.read(reinterpret_cast<char*>(output.data()), size);
  }
  return static_cast<bool>(stream);
}

std::filesystem::path nativePath(const std::string_view virtualPath) {
  std::filesystem::path result;
  std::size_t begin = 0;
  while (begin < virtualPath.size()) {
    const std::size_t separator = virtualPath.find_first_of("\\/", begin);
    const std::size_t end = separator == std::string_view::npos ? virtualPath.size() : separator;
    if (end != begin) {
      result /= std::string(virtualPath.substr(begin, end - begin));
    }
    if (separator == std::string_view::npos) {
      break;
    }
    begin = separator + 1U;
  }
  return result;
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

} // namespace

int main(const int argc, const char* const* argv) {
  if (argc != 5 && argc != 6) {
    std::cerr << "usage: archive_material_probe <file.pak> <packlist.dat> <logical-solid> "
                 "<GameData-root> [identifier]\n";
    return 2;
  }
  const std::filesystem::path archivePath(argv[1]);
  const std::filesystem::path gameDataRoot(argv[4]);
  std::vector<std::byte> archiveBytes;
  std::vector<std::byte> packListBytes;
  if (!readFile(archivePath, archiveBytes) || !readFile(argv[2], packListBytes)) {
    std::cerr << "unable to read archive or packlist\n";
    return 2;
  }

  tmnf::pak::VirtualFileSystem vfs;
  const tmnf::pak::VfsOperationResult initialized = vfs.initialize(packListBytes);
  if (!initialized) {
    std::cerr << initialized.error << '\n';
    return 1;
  }
  const std::string identifier = argc == 6 ? argv[5] : archivePath.stem().string();
  const tmnf::pak::VfsOperationResult mounted = vfs.mount(identifier, std::move(archiveBytes));
  if (!mounted) {
    std::cerr << mounted.error << '\n';
    return 1;
  }
  tmnf::pak::VfsOpenResult opened = vfs.openFile(argv[3]);
  if (!opened) {
    std::cerr << opened.error << '\n';
    return 1;
  }
  tmnf::gbx::SolidParseResult parsed = tmnf::gbx::parseSolid(*opened.input);
  if (!parsed) {
    std::cerr << "unable to parse packed solid: " << parsed.error << '\n';
    return 1;
  }

  const tmnf::content::LooseFileReader looseReader =
      [&gameDataRoot](const std::string_view path) -> std::optional<std::vector<std::byte>> {
    std::vector<std::byte> bytes;
    if (!readFile(gameDataRoot / nativePath(path), bytes)) {
      return std::nullopt;
    }
    return bytes;
  };
  tmnf::content::SolidMaterialResolveResult resolved =
      tmnf::content::resolveSolidMaterials(parsed.solid, vfs, looseReader);
  if (!resolved) {
    std::cerr << "unable to resolve solid materials: " << resolved.error << '\n';
    return 1;
  }

  std::cout << "{\n  \"textureCount\": " << resolved.materials.textures.size()
            << ",\n  \"visualTextureIndices\": [";
  for (std::size_t index = 0; index < resolved.materials.visualTextureIndices.size(); ++index) {
    if (index != 0) {
      std::cout << ", ";
    }
    const std::size_t textureIndex = resolved.materials.visualTextureIndices[index];
    if (textureIndex == tmnf::content::kNoVisualTexture) {
      std::cout << "null";
    } else {
      std::cout << textureIndex;
    }
  }
  std::cout << "],\n  \"textures\": [\n";
  for (std::size_t index = 0; index < resolved.materials.textures.size(); ++index) {
    const tmnf::content::ResolvedMaterialTexture& texture = resolved.materials.textures[index];
    const std::span pixelBytes(reinterpret_cast<const std::byte*>(texture.image.pixels.data()),
                               texture.image.pixels.size());
    std::cout << "    {\"material\": \"" << jsonEscape(texture.materialReference)
              << "\", \"textureDocument\": \"" << jsonEscape(texture.textureDocumentPath)
              << "\", \"image\": \"" << jsonEscape(texture.imagePath)
              << "\", \"width\": " << texture.image.width
              << ", \"height\": " << texture.image.height << ", \"pixelMd5\": \""
              << tmnf::crypto::toHex(tmnf::crypto::md5(pixelBytes)) << "\"}"
              << (index + 1U == resolved.materials.textures.size() ? "" : ",") << '\n';
  }
  std::cout << "  ]\n}\n";
  return 0;
}
