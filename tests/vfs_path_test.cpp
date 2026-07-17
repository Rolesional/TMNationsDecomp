#include "tmnf/pak/vfs.hpp"

#include <iostream>

int main() {
  if (tmnf::pak::normalizeArchivePath("/Media//Texture/Image/WhiteFlare1.TGA/") !=
          "media\\texture\\image\\whiteflare1.tga" ||
      tmnf::pak::normalizeArchivePath("RESOURCE") != "resource" ||
      !tmnf::pak::normalizeArchivePath("////").empty() ||
      tmnf::pak::nadeoPathHash("Tube\\Road\\BaseGround.Solid.Gbx") !=
          "E11CFB84BECB8590C7832182A5B1718D70" ||
      tmnf::pak::nadeoPathHash("TUBE/ROAD/BASEGROUND.SOLID.GBX") !=
          "E11CFB84BECB8590C7832182A5B1718D70") {
    std::cerr << "archive path normalization failed\n";
    return 1;
  }

  tmnf::pak::VirtualFileSystem vfs;
  if (vfs.contains("missing") || vfs.mountCount() != 0 || vfs.fileCount() != 0) {
    std::cerr << "empty VFS state was invalid\n";
    return 1;
  }
  return 0;
}
