#include "tmnf/pak/vfs.hpp"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
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

} // namespace

int main(const int argc, const char* const* argv) {
  if (argc != 5 && argc != 6) {
    std::cerr << "usage: archive_extract <file.pak> <packlist.dat> <entry> <output> "
                 "[identifier]\n";
    return 2;
  }

  const std::filesystem::path archivePath(argv[1]);
  const std::filesystem::path packListPath(argv[2]);
  const std::string requestedPath(argv[3]);
  const std::filesystem::path outputPath(argv[4]);
  const std::string identifier = argc == 6 ? argv[5] : archivePath.stem().string();

  std::vector<std::byte> archiveRaw;
  std::vector<std::byte> packListRaw;
  if (!readFile(archivePath, archiveRaw) || !readFile(packListPath, packListRaw)) {
    std::cerr << "unable to read archive or packlist\n";
    return 2;
  }

  tmnf::pak::VirtualFileSystem vfs;
  const tmnf::pak::VfsOperationResult initialized = vfs.initialize(packListRaw);
  if (!initialized) {
    std::cerr << initialized.error << '\n';
    return 1;
  }
  const tmnf::pak::VfsOperationResult mounted = vfs.mount(identifier, std::move(archiveRaw));
  if (!mounted) {
    std::cerr << mounted.error << '\n';
    return 1;
  }
  tmnf::pak::VfsReadResult data = vfs.readFile(requestedPath);
  if (!data) {
    std::cerr << data.error << '\n';
    return 1;
  }
  if (outputPath.has_parent_path()) {
    std::filesystem::create_directories(outputPath.parent_path());
  }
  std::ofstream output(outputPath, std::ios::binary);
  if (!output) {
    std::cerr << "unable to create: " << outputPath << '\n';
    return 2;
  }
  output.write(reinterpret_cast<const char*>(data.data.data()),
               static_cast<std::streamsize>(data.data.size()));
  if (!output) {
    std::cerr << "unable to write: " << outputPath << '\n';
    return 2;
  }
  std::cout << "extracted " << data.data.size() << " bytes to " << outputPath << '\n';
  return 0;
}
