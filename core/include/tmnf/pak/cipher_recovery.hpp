#pragma once

#include "tmnf/crypto/md5.hpp"
#include "tmnf/pak/archive.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace tmnf::pak {

struct CipherRecoveryResult final {
  std::vector<std::byte> data;
  std::vector<std::uint64_t> cycleXors;
  std::size_t candidateCount = 0;
  std::size_t exploredStateCount = 0;
  bool uniquenessProven = false;
  std::string error;

  [[nodiscard]] explicit operator bool() const noexcept { return error.empty(); }
};

// Research fallback for archive-compressed GBX files whose nested node parser
// is not implemented yet. It tries zero through three recovered class seeds at
// each 256-byte cipher boundary and uses the zlib state/checksum as an oracle.
// A zero-first depth-first search avoids retaining every ambiguous intermediate
// zlib state. The resulting cycle XORs can be cached as compact runtime metadata.
[[nodiscard]] CipherRecoveryResult
recoverFileDataByDeflateOracle(std::span<const std::byte> archiveBytes, const Archive& archive,
                               const FileEntry& file, const crypto::Md5Digest& archiveKey,
                               std::size_t maximumSeedWrites = 3,
                               std::size_t maximumSearchStates = 500'000);

} // namespace tmnf::pak
