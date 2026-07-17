#include "tmnf/pak/cipher_recovery.hpp"

#include "tmnf/gbx/class_registry.hpp"
#include "tmnf/pak/file_data.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

extern "C" {

using RecoveryZByte = unsigned char;
using RecoveryZUInt = unsigned int;
using RecoveryZULong = unsigned long;
using RecoveryZVoid = void*;
using RecoveryZAlloc = RecoveryZVoid (*)(RecoveryZVoid opaque, RecoveryZUInt items,
                                         RecoveryZUInt size);
using RecoveryZFree = void (*)(RecoveryZVoid opaque, RecoveryZVoid address);

struct RecoveryZInternalState;
struct RecoveryZStream {
  RecoveryZByte* nextIn;
  RecoveryZUInt availableIn;
  RecoveryZULong totalIn;
  RecoveryZByte* nextOut;
  RecoveryZUInt availableOut;
  RecoveryZULong totalOut;
  char* message;
  RecoveryZInternalState* state;
  RecoveryZAlloc allocate;
  RecoveryZFree free;
  RecoveryZVoid opaque;
  int dataType;
  RecoveryZULong adler;
  RecoveryZULong reserved;
};

const char* zlibVersion();
int inflateInit_(RecoveryZStream* stream, const char* version, int streamSize);
int inflateCopy(RecoveryZStream* destination, RecoveryZStream* source);
int inflate(RecoveryZStream* stream, int flush);
int inflateEnd(RecoveryZStream* stream);

} // extern "C"

namespace tmnf::pak {
namespace {

constexpr int kZlibNoFlush = 0;
constexpr int kZlibOk = 0;
constexpr int kZlibStreamEnd = 1;
constexpr std::size_t kCipherCycleSize = 0x100;

struct Branch final {
  RecoveryZStream stream{};
  std::vector<std::byte> output;
  std::vector<std::uint64_t> corrections;
  bool initialized = false;
  bool ended = false;

  Branch() = default;

  ~Branch() {
    if (initialized) {
      inflateEnd(&stream);
    }
  }

  Branch(const Branch&) = delete;
  Branch& operator=(const Branch&) = delete;
};

std::unique_ptr<Branch> createBranch(const std::size_t outputSize, std::string& error) {
  auto branch = std::make_unique<Branch>();
  branch->output.resize(outputSize);
  const int status =
      inflateInit_(&branch->stream, zlibVersion(), static_cast<int>(sizeof(RecoveryZStream)));
  if (status != kZlibOk) {
    error = "unable to initialize zlib recovery state";
    return nullptr;
  }
  branch->initialized = true;
  return branch;
}

std::unique_ptr<Branch> cloneBranch(Branch& source, std::string& error) {
  auto branch = std::make_unique<Branch>();
  branch->output = source.output;
  branch->corrections = source.corrections;
  branch->ended = source.ended;
  const int status = inflateCopy(&branch->stream, &source.stream);
  if (status != kZlibOk) {
    error = "unable to clone zlib recovery state";
    return nullptr;
  }
  branch->initialized = true;
  return branch;
}

std::uint64_t applyInitializationByte(std::uint64_t value, const std::uint8_t byte) noexcept {
  return std::rotl(value, 13) ^ static_cast<std::uint64_t>(byte | 0xaaU);
}

std::uint64_t applyClassSeed(std::uint64_t value, const std::uint32_t seed) noexcept {
  for (std::size_t index = 0; index < 4; ++index) {
    value =
        applyInitializationByte(value, static_cast<std::uint8_t>((seed >> (index * 8U)) & 0xffU));
  }
  return value;
}

std::vector<std::uint32_t> classSeeds() {
  std::vector<std::uint32_t> seeds;
  for (const gbx::ClassInfo& info : gbx::classRegistry()) {
    const std::optional<std::uint32_t> seed = gbx::dummyWriteSeedClassId(info.id);
    if (seed.has_value()) {
      seeds.push_back(*seed);
    }
  }
  std::sort(seeds.begin(), seeds.end());
  seeds.erase(std::unique(seeds.begin(), seeds.end()), seeds.end());
  return seeds;
}

std::vector<std::uint64_t> candidateXors(const std::size_t maximumSeedWrites) {
  std::vector<std::uint64_t> candidates{0};
  const std::vector<std::uint32_t> seeds = classSeeds();
  std::unordered_set<std::uint64_t> emitted{0};
  std::vector<std::uint64_t> frontier{0};
  for (std::size_t depth = 0; depth < maximumSeedWrites; ++depth) {
    std::vector<std::uint64_t> nextFrontier;
    std::unordered_set<std::uint64_t> nextEmitted;
    for (const std::uint64_t previous : frontier) {
      for (const std::uint32_t seed : seeds) {
        const std::uint64_t value = applyClassSeed(previous, seed);
        if (nextEmitted.insert(value).second) {
          nextFrontier.push_back(value);
        }
        if (emitted.insert(value).second) {
          candidates.push_back(value);
        }
      }
    }
    frontier = std::move(nextFrontier);
  }
  return candidates;
}

void applyCorrection(std::span<std::byte> cycle, const std::uint64_t correction) noexcept {
  const std::size_t count = std::min<std::size_t>(8, cycle.size());
  for (std::size_t index = 0; index < count; ++index) {
    cycle[index] ^= static_cast<std::byte>((correction >> (index * 8U)) & 0xffU);
  }
}

bool feed(Branch& branch, std::span<std::byte> input) {
  if (branch.ended) {
    return true;
  }
  branch.stream.nextIn = reinterpret_cast<RecoveryZByte*>(input.data());
  branch.stream.availableIn = static_cast<RecoveryZUInt>(input.size());
  std::byte extra{};

  while (branch.stream.availableIn != 0 && !branch.ended) {
    const RecoveryZUInt inputBefore = branch.stream.availableIn;
    const RecoveryZULong outputBefore = branch.stream.totalOut;
    const std::size_t outputPosition = static_cast<std::size_t>(branch.stream.totalOut);
    const bool outputFull = outputPosition >= branch.output.size();
    if (outputFull) {
      branch.stream.nextOut = reinterpret_cast<RecoveryZByte*>(&extra);
      branch.stream.availableOut = 1;
    } else {
      const std::size_t available =
          std::min(branch.output.size() - outputPosition,
                   static_cast<std::size_t>(std::numeric_limits<RecoveryZUInt>::max()));
      branch.stream.nextOut =
          reinterpret_cast<RecoveryZByte*>(branch.output.data() + outputPosition);
      branch.stream.availableOut = static_cast<RecoveryZUInt>(available);
    }

    const int status = inflate(&branch.stream, kZlibNoFlush);
    if (outputFull && branch.stream.availableOut == 0) {
      return false;
    }
    if (status == kZlibStreamEnd) {
      branch.ended = true;
      break;
    }
    if (status != kZlibOk ||
        (inputBefore == branch.stream.availableIn && outputBefore == branch.stream.totalOut)) {
      return false;
    }
  }
  return branch.ended || branch.stream.availableIn == 0;
}

struct SearchContext final {
  std::span<const std::byte> stored;
  std::span<const std::uint64_t> candidates;
  std::size_t cycleCount = 0;
  std::size_t expectedOutputSize = 0;
  std::size_t maximumStates = 0;
  std::size_t exploredStates = 0;
  bool limitReached = false;
  std::string error;
  std::vector<std::unique_ptr<Branch>> solutions;
};

void searchCycles(SearchContext& context, const std::size_t cycleIndex,
                  std::unique_ptr<Branch> branch) {
  if (!context.error.empty() || context.limitReached || context.solutions.size() > 1) {
    return;
  }
  if (branch->ended || cycleIndex == context.cycleCount) {
    if (branch->ended &&
        branch->stream.totalOut == static_cast<RecoveryZULong>(context.expectedOutputSize)) {
      branch->corrections.resize(context.cycleCount, 0);
      context.solutions.push_back(std::move(branch));
    }
    return;
  }

  const std::size_t offset = cycleIndex * kCipherCycleSize;
  const std::size_t length = std::min(kCipherCycleSize, context.stored.size() - offset);
  const std::span<const std::byte> sourceCycle = context.stored.subspan(offset, length);
  const std::span<const std::uint64_t> cycleCandidates =
      cycleIndex == 0 ? context.candidates.first(1) : context.candidates;

  for (const std::uint64_t correction : cycleCandidates) {
    if (context.exploredStates == context.maximumStates) {
      context.limitReached = true;
      return;
    }
    ++context.exploredStates;

    std::unique_ptr<Branch> candidate = cloneBranch(*branch, context.error);
    if (candidate == nullptr) {
      return;
    }
    std::array<std::byte, kCipherCycleSize> patched{};
    std::copy(sourceCycle.begin(), sourceCycle.end(), patched.begin());
    applyCorrection(std::span(patched).first(length), correction);
    candidate->corrections.push_back(correction);
    if (!feed(*candidate, std::span(patched).first(length))) {
      continue;
    }
    searchCycles(context, cycleIndex + 1, std::move(candidate));
    if (!context.error.empty() || context.limitReached || context.solutions.size() > 1) {
      return;
    }
  }
}

CipherRecoveryResult failed(std::string message, const std::size_t candidateCount = 0,
                            const std::size_t exploredStateCount = 0) {
  CipherRecoveryResult result;
  result.candidateCount = candidateCount;
  result.exploredStateCount = exploredStateCount;
  result.error = std::move(message);
  return result;
}

} // namespace

CipherRecoveryResult recoverFileDataByDeflateOracle(const std::span<const std::byte> archiveBytes,
                                                    const Archive& archive, const FileEntry& file,
                                                    const crypto::Md5Digest& archiveKey,
                                                    const std::size_t maximumSeedWrites,
                                                    const std::size_t maximumSearchStates) {
  if (!file.compressed()) {
    return failed("deflate-oracle recovery requires an archive-compressed file");
  }
  if (maximumSeedWrites > 3) {
    return failed("deflate-oracle recovery currently supports at most three seed writes per cycle");
  }
  if (maximumSearchStates == 0) {
    return failed("deflate-oracle recovery requires a positive search-state limit");
  }

  FileDataResult stored =
      readStoredFileDataWithoutDummyWrites(archiveBytes, archive, file, archiveKey);
  if (!stored) {
    return failed(std::move(stored.error));
  }
  const std::vector<std::uint64_t> candidates = candidateXors(maximumSeedWrites);

  std::string zlibError;
  std::unique_ptr<Branch> root =
      createBranch(static_cast<std::size_t>(file.uncompressedSize), zlibError);
  if (root == nullptr) {
    return failed(std::move(zlibError), candidates.size());
  }

  const std::size_t cycleCount = (stored.data.size() + kCipherCycleSize - 1U) / kCipherCycleSize;
  SearchContext context{
      .stored = stored.data,
      .candidates = candidates,
      .cycleCount = cycleCount,
      .expectedOutputSize = static_cast<std::size_t>(file.uncompressedSize),
      .maximumStates = maximumSearchStates,
      .exploredStates = 0,
      .limitReached = false,
      .error = {},
      .solutions = {},
  };
  searchCycles(context, 0, std::move(root));
  if (!context.error.empty()) {
    return failed(std::move(context.error), candidates.size(), context.exploredStates);
  }
  if (context.solutions.empty()) {
    const std::string message =
        context.limitReached
            ? "deflate-oracle recovery reached the search-state limit before finding a solution"
            : "no dummy-write candidate produced a complete zlib stream";
    return failed(message, candidates.size(), context.exploredStates);
  }
  if (context.solutions.size() != 1) {
    return failed("multiple dummy-write candidates produced complete zlib streams",
                  candidates.size(), context.exploredStates);
  }

  CipherRecoveryResult result;
  result.data = std::move(context.solutions.front()->output);
  result.cycleXors = std::move(context.solutions.front()->corrections);
  result.candidateCount = candidates.size();
  result.exploredStateCount = context.exploredStates;
  result.uniquenessProven = !context.limitReached;
  return result;
}

} // namespace tmnf::pak
