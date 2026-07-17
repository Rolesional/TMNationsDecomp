#include "tmnf/compression/lzo.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <span>
#include <string>
#include <utility>

// The instruction decoder is adapted from AxioDL/lzokay (MIT), commit
// db2df1fcbebc2ed06c10f727f72567d40f06a2be. See THIRD_PARTY_NOTICES.md.

namespace tmnf::compression {
namespace {

class Decoder final {
public:
  Decoder(const std::span<const std::byte> input, const std::size_t outputSize)
      : input_(input), output_(outputSize) {}

  LzoDecompressResult run() {
    if (input_.size() < 3) {
      return fail("LZO input is shorter than the end marker");
    }

    std::size_t state = 0;
    const std::uint8_t first = peek();
    if (first >= 22) {
      ++inputPosition_;
      if (!copyLiterals(static_cast<std::size_t>(first) - 17U)) {
        return takeFailure();
      }
      state = 4;
    } else if (first >= 18) {
      ++inputPosition_;
      state = static_cast<std::size_t>(first) - 17U;
      if (!copyLiterals(state)) {
        return takeFailure();
      }
    }

    bool foundEndMarker = false;
    std::size_t terminatingLength = 0;
    while (!foundEndMarker) {
      std::uint8_t instruction = 0;
      if (!readByte(instruction)) {
        return takeFailure();
      }

      std::size_t matchDistance = 0;
      std::size_t matchLength = 0;
      std::size_t nextState = 0;
      if ((instruction & 0xc0U) != 0) {
        std::uint8_t highDistance = 0;
        if (!readByte(highDistance)) {
          return takeFailure();
        }
        matchDistance =
            (static_cast<std::size_t>(highDistance) << 3U) + ((instruction >> 2U) & 0x7U) + 1U;
        matchLength = static_cast<std::size_t>(instruction >> 5U) + 1U;
        nextState = instruction & 0x3U;
      } else if ((instruction & 0x20U) != 0) {
        matchLength = static_cast<std::size_t>(instruction & 0x1fU) + 2U;
        if (matchLength == 2 && !extendLength(31U, matchLength)) {
          return takeFailure();
        }
        std::uint16_t encoded = 0;
        if (!readLe16(encoded)) {
          return takeFailure();
        }
        matchDistance = (static_cast<std::size_t>(encoded) >> 2U) + 1U;
        nextState = encoded & 0x3U;
      } else if ((instruction & 0x10U) != 0) {
        matchLength = static_cast<std::size_t>(instruction & 0x7U) + 2U;
        if (matchLength == 2 && !extendLength(7U, matchLength)) {
          return takeFailure();
        }
        std::uint16_t encoded = 0;
        if (!readLe16(encoded)) {
          return takeFailure();
        }
        const std::size_t encodedDistance = (static_cast<std::size_t>(instruction & 0x8U) << 11U) +
                                            (static_cast<std::size_t>(encoded) >> 2U);
        nextState = encoded & 0x3U;
        if (encodedDistance == 0) {
          terminatingLength = matchLength;
          foundEndMarker = true;
          continue;
        }
        matchDistance = encodedDistance + 16384U;
      } else if (state == 0) {
        std::size_t literalLength = static_cast<std::size_t>(instruction) + 3U;
        if (literalLength == 3 && !extendLength(15U, literalLength)) {
          return takeFailure();
        }
        if (!copyLiterals(literalLength)) {
          return takeFailure();
        }
        state = 4;
        continue;
      } else {
        std::uint8_t highDistance = 0;
        if (!readByte(highDistance)) {
          return takeFailure();
        }
        nextState = instruction & 0x3U;
        if (state != 4) {
          matchDistance = (instruction >> 2U) + (static_cast<std::size_t>(highDistance) << 2U) + 1U;
          matchLength = 2;
        } else {
          matchDistance =
              (instruction >> 2U) + (static_cast<std::size_t>(highDistance) << 2U) + 2049U;
          matchLength = 3;
        }
      }

      if (matchDistance == 0 || matchDistance > outputPosition_) {
        return fail("LZO match exceeds the available lookbehind window");
      }
      if (!requireOutput(matchLength + nextState) || !requireInput(nextState)) {
        return takeFailure();
      }
      std::size_t source = outputPosition_ - matchDistance;
      for (std::size_t index = 0; index < matchLength; ++index) {
        output_[outputPosition_++] = output_[source++];
      }
      state = nextState;
      if (!copyLiterals(nextState)) {
        return takeFailure();
      }
    }

    if (terminatingLength != 3) {
      return fail("LZO stream has an invalid end marker");
    }
    if (inputPosition_ != input_.size()) {
      return fail("LZO stream has trailing compressed bytes");
    }
    if (outputPosition_ != output_.size()) {
      return fail("LZO output size does not match the declared GBX body size");
    }

    LzoDecompressResult result;
    result.data = std::move(output_);
    return result;
  }

private:
  std::uint8_t peek() const noexcept {
    return std::to_integer<std::uint8_t>(input_[inputPosition_]);
  }

  bool requireInput(const std::size_t count) {
    if (inputPosition_ > input_.size() || count > input_.size() - inputPosition_) {
      setError("LZO instruction overruns its input");
      return false;
    }
    return true;
  }

  bool requireOutput(const std::size_t count) {
    if (outputPosition_ > output_.size() || count > output_.size() - outputPosition_) {
      setError("LZO instruction overruns its output buffer");
      return false;
    }
    return true;
  }

  bool readByte(std::uint8_t& value) {
    if (!requireInput(1)) {
      return false;
    }
    value = peek();
    ++inputPosition_;
    return true;
  }

  bool readLe16(std::uint16_t& value) {
    std::uint8_t low = 0;
    std::uint8_t high = 0;
    if (!readByte(low) || !readByte(high)) {
      return false;
    }
    value = static_cast<std::uint16_t>(low) |
            static_cast<std::uint16_t>(static_cast<std::uint16_t>(high) << 8U);
    return true;
  }

  bool copyLiterals(const std::size_t count) {
    if (!requireInput(count) || !requireOutput(count)) {
      return false;
    }
    std::copy_n(input_.begin() + static_cast<std::ptrdiff_t>(inputPosition_), count,
                output_.begin() + static_cast<std::ptrdiff_t>(outputPosition_));
    inputPosition_ += count;
    outputPosition_ += count;
    return true;
  }

  bool extendLength(const std::size_t base, std::size_t& length) {
    std::size_t zeroCount = 0;
    while (true) {
      if (!requireInput(1)) {
        return false;
      }
      if (peek() != 0) {
        break;
      }
      ++inputPosition_;
      ++zeroCount;
    }
    std::uint8_t tail = 0;
    if (!readByte(tail)) {
      return false;
    }
    constexpr std::size_t maximum = std::numeric_limits<std::size_t>::max();
    if (zeroCount > (maximum - base - tail) / 255U) {
      setError("LZO extended length overflows the platform size type");
      return false;
    }
    const std::size_t extension = zeroCount * 255U + base + tail;
    if (length > maximum - extension) {
      setError("LZO match length overflows the platform size type");
      return false;
    }
    length += extension;
    return true;
  }

  void setError(std::string message) {
    if (error_.empty()) {
      error_ = std::move(message);
    }
  }

  LzoDecompressResult fail(std::string message) {
    setError(std::move(message));
    return takeFailure();
  }

  LzoDecompressResult takeFailure() {
    LzoDecompressResult result;
    result.error = std::move(error_);
    if (result.error.empty()) {
      result.error = "unable to decompress LZO1X stream";
    }
    return result;
  }

  std::span<const std::byte> input_;
  std::vector<std::byte> output_;
  std::size_t inputPosition_ = 0;
  std::size_t outputPosition_ = 0;
  std::string error_;
};

} // namespace

LzoDecompressResult decompressLzo1x(const std::span<const std::byte> compressed,
                                    const std::size_t expectedSize) {
  return Decoder(compressed, expectedSize).run();
}

} // namespace tmnf::compression
