#include "tmnf/gbx/sound_engine.hpp"

#include "tmnf/gbx/document.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace tmnf::gbx {
namespace {

constexpr std::uint32_t kFacade = 0xFACADE01U;
constexpr std::uint32_t kSkip = 0x534B4950U;
constexpr std::uint32_t kSoundEngineClassId = 0x0908E000U;
constexpr std::uint32_t kSoundEngineComponentClassId = 0x0908F000U;
constexpr std::uint32_t kComponentSeed = 0x01001000U;
constexpr std::uint32_t kCurveSeed = 0x05002000U;
constexpr std::int32_t kMaximumCount = 1 << 20;

class Reader final {
public:
  explicit Reader(Input& input) : input_(input) {}

  bool readBytes(const std::span<std::byte> destination) {
    if (input_.position() > input_.size() ||
        destination.size() > input_.size() - input_.position()) {
      return fail("sound-engine read exceeds the declared GBX size");
    }
    if (!input_.readBytes(destination)) {
      const std::string_view detail = input_.error();
      return fail(detail.empty() ? "unable to read sound-engine GBX" : std::string(detail));
    }
    return true;
  }

  bool readU32(std::uint32_t& value) {
    std::array<std::byte, 4> bytes{};
    if (!readBytes(bytes)) {
      return false;
    }
    value = 0;
    for (std::size_t index = 0; index < bytes.size(); ++index) {
      value |= static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(bytes[index]))
               << (index * 8U);
    }
    return true;
  }

  bool readI32(std::int32_t& value) {
    std::uint32_t encoded = 0;
    if (!readU32(encoded)) {
      return false;
    }
    value = std::bit_cast<std::int32_t>(encoded);
    return true;
  }

  bool readFloat(float& value) {
    std::uint32_t encoded = 0;
    if (!readU32(encoded)) {
      return false;
    }
    value = std::bit_cast<float>(encoded);
    return std::isfinite(value) || fail("sound engine contains a non-finite float");
  }

  bool readCount(std::int32_t& count, const char* const description) {
    if (!readI32(count) || count < 0 || count > kMaximumCount) {
      return fail(std::string("invalid sound-engine ") + description + " count");
    }
    return true;
  }

  bool readString(std::string& value) {
    std::int32_t count = 0;
    if (!readCount(count, "string")) {
      return false;
    }
    value.resize(static_cast<std::size_t>(count));
    return readBytes(std::span(reinterpret_cast<std::byte*>(value.data()), value.size()));
  }

  bool readFloatArray(std::vector<float>& values) {
    std::int32_t count = 0;
    if (!readCount(count, "float-array")) {
      return false;
    }
    values.resize(static_cast<std::size_t>(count));
    for (float& value : values) {
      if (!readFloat(value)) {
        return false;
      }
    }
    return true;
  }

  bool skip(std::size_t count) {
    std::array<std::byte, 256> buffer{};
    while (count != 0U) {
      const std::size_t chunk = std::min(count, buffer.size());
      if (!readBytes(std::span(buffer).first(chunk))) {
        return false;
      }
      count -= chunk;
    }
    return true;
  }

  bool fail(std::string message) {
    if (error_.empty()) {
      error_ = std::move(message);
    }
    return false;
  }

  [[nodiscard]] std::string takeError() { return std::move(error_); }

private:
  Input& input_;
  std::string error_;
};

bool validCurve(const RealCurve& curve) noexcept {
  return !curve.x.empty() && curve.x.size() == curve.y.size() &&
         std::ranges::is_sorted(curve.x) &&
         std::ranges::all_of(curve.x, [](const float value) { return std::isfinite(value); }) &&
         std::ranges::all_of(curve.y, [](const float value) { return std::isfinite(value); });
}

bool validComponent(const SoundEngineComponent& component) noexcept {
  return !component.soundPath.empty() && component.minVolume >= 0.0F &&
         component.maxVolume >= component.minVolume && component.minPitch > 0.0F &&
         component.maxPitch >= component.minPitch;
}

float sampleCurve(const RealCurve& curve, const float coordinate) noexcept {
  if (!validCurve(curve) || !std::isfinite(coordinate)) {
    return 0.0F;
  }
  if (coordinate <= curve.x.front()) {
    return curve.y.front();
  }
  if (coordinate >= curve.x.back()) {
    return curve.y.back();
  }
  const auto right = std::upper_bound(curve.x.begin(), curve.x.end(), coordinate);
  const std::size_t rightIndex = static_cast<std::size_t>(right - curve.x.begin());
  const std::size_t leftIndex = rightIndex - 1U;
  const float span = curve.x[rightIndex] - curve.x[leftIndex];
  return span <= 0.0F
             ? curve.y[rightIndex]
             : std::lerp(curve.y[leftIndex], curve.y[rightIndex],
                         (coordinate - curve.x[leftIndex]) / span);
}

class SoundEngineParser final {
public:
  SoundEngineParser(Input& input, const DocumentProbe& document)
      : input_(input), reader_(input), document_(document) {
    for (const ExternalNode& node : document.externalNodes) {
      externalPaths_.emplace(node.nodeIndex, externalNodePath(document, node));
    }
  }

  SoundEngineParseResult run() {
    if (document_.header.classId != kSoundEngineClassId ||
        document_.firstChunkId != 0x09001001U) {
      return failed("GBX document is not an uncompressed legacy sound engine");
    }

    std::string ignoredId;
    if (!readId(ignoredId)) {
      return failedReader("invalid sound-engine base ID");
    }
    while (true) {
      std::uint32_t chunkId = 0;
      if (!reader_.readU32(chunkId)) {
        return failedReader("truncated sound-engine body");
      }
      switch (chunkId) {
      case kFacade:
        if (input_.position() != input_.size()) {
          return failed("sound-engine root ended before EOF");
        }
        return finish();
      case 0x0901A000U:
      case 0x0901A002U:
        if (!readId(ignoredId)) {
          return failedReader("invalid sound-engine base reference");
        }
        break;
      case 0x0901A00BU:
        if (!reader_.skip(5U * sizeof(std::uint32_t))) {
          return failedReader("truncated sound-engine cone settings");
        }
        break;
      case 0x0901A00CU:
        if (!reader_.skip(11U * sizeof(std::uint32_t))) {
          return failedReader("truncated sound-engine distance settings");
        }
        break;
      case 0x0901A00DU:
        if (!readPlaybackSettings()) {
          return failedReader("truncated sound-engine playback settings");
        }
        break;
      case kSoundEngineClassId:
        if (!readComponents()) {
          return failed(reader_.takeError());
        }
        break;
      case 0x0908E001U:
        if (!reader_.readFloat(engine_.maxRpm)) {
          return failedReader("invalid sound-engine maximum RPM");
        }
        break;
      case 0x0908E002U:
        if (!readCurves()) {
          return failed(reader_.takeError());
        }
        break;
      default:
        if (!skipSkippableChunk(chunkId)) {
          return failed(reader_.takeError());
        }
        break;
      }
    }
  }

private:
  SoundEngineParseResult failed(std::string message) {
    SoundEngineParseResult result;
    result.error = std::move(message);
    if (result.error.empty()) {
      result.error = "unable to parse legacy sound engine";
    }
    return result;
  }

  SoundEngineParseResult failedReader(const char* const fallback) {
    std::string error = reader_.takeError();
    return failed(error.empty() ? std::string(fallback) : std::move(error));
  }

  SoundEngineParseResult finish() {
    if (engine_.components.empty() ||
        !std::ranges::all_of(engine_.components, validComponent) ||
        !std::isfinite(engine_.volume) || engine_.volume < 0.0F ||
        !std::isfinite(engine_.maxRpm) || engine_.maxRpm <= 0.0F ||
        !std::ranges::all_of(engine_.volumeCurves, validCurve) ||
        !std::ranges::all_of(engine_.alphaCurves, validCurve)) {
      return failed("sound engine is incomplete");
    }
    SoundEngineParseResult result;
    result.engine = std::move(engine_);
    return result;
  }

  void initializeU32(const std::uint32_t value) {
    std::array<std::byte, 4> bytes{};
    for (std::size_t index = 0; index < bytes.size(); ++index) {
      bytes[index] = static_cast<std::byte>((value >> (index * 8U)) & 0xffU);
    }
    input_.initialize(bytes);
  }

  bool readId(std::string& value) {
    if (!idVersionRead_) {
      std::int32_t version = 0;
      if (!reader_.readI32(version) || version != 3) {
        return reader_.fail("unsupported GBX ID version in sound engine");
      }
      idVersionRead_ = true;
    }
    std::uint32_t encoded = 0;
    if (!reader_.readU32(encoded)) {
      return false;
    }
    if (encoded == std::numeric_limits<std::uint32_t>::max()) {
      value.clear();
      return true;
    }
    const std::uint32_t tag = encoded & 0xC0000000U;
    if (tag != 0x40000000U && tag != 0x80000000U) {
      value = std::to_string(encoded);
      return true;
    }
    if ((encoded & 0x3FFFFFFFU) != 0U) {
      const auto found = ids_.find(encoded);
      if (found == ids_.end()) {
        return reader_.fail("sound engine references an unknown GBX ID");
      }
      value = found->second;
      return true;
    }
    if (!reader_.readString(value)) {
      return false;
    }
    const std::uint32_t key = encoded + static_cast<std::uint32_t>(ids_.size()) + 1U;
    return ids_.emplace(key, value).second || reader_.fail("duplicate sound-engine GBX ID");
  }

  bool readComponents() {
    std::int32_t count = 0;
    if (!reader_.readCount(count, "component") || count == 0 || count > 64) {
      return reader_.fail("invalid sound-engine component count");
    }
    engine_.components.reserve(static_cast<std::size_t>(count));
    for (std::int32_t componentIndex = 0; componentIndex < count; ++componentIndex) {
      std::int32_t nodeIndex = -1;
      std::uint32_t classId = 0;
      std::uint32_t chunkId = 0;
      if (!reader_.readI32(nodeIndex) || nodeIndex < 0 || componentNodes_.contains(nodeIndex) ||
          !reader_.readU32(classId) || classId != kSoundEngineComponentClassId) {
        return reader_.fail("invalid inline sound-engine component reference");
      }
      componentNodes_.emplace(nodeIndex, engine_.components.size());
      initializeU32(kComponentSeed);
      if (!reader_.readU32(chunkId) || chunkId != kSoundEngineComponentClassId) {
        return reader_.fail("invalid sound-engine component chunk");
      }
      SoundEngineComponent component;
      std::int32_t soundNodeIndex = -1;
      if (!reader_.readI32(soundNodeIndex)) {
        return false;
      }
      const auto sound = externalPaths_.find(soundNodeIndex);
      if (sound == externalPaths_.end() || sound->second.empty()) {
        return reader_.fail("sound-engine component references an unknown sound");
      }
      component.soundPath = sound->second;
      float* const fields[] = {
          &component.minVolume,          &component.maxVolume,
          &component.fadeInStartRpm,     &component.fadeInEndRpm,
          &component.fadeOutStartRpm,    &component.fadeOutEndRpm,
          &component.minPitch,           &component.maxPitch,
          &component.pitchShiftStartRpm, &component.pitchShiftEndRpm,
      };
      for (float* const field : fields) {
        if (!reader_.readFloat(*field)) {
          return false;
        }
      }
      std::uint32_t facade = 0;
      if (!reader_.readU32(facade) || facade != kFacade) {
        return reader_.fail("invalid sound-engine component terminator");
      }
      engine_.components.push_back(std::move(component));
    }
    return true;
  }

  bool readPlaybackSettings() {
    std::int32_t mode = 0;
    std::int32_t looping = 0;
    std::int32_t continuous = 0;
    float priority = 0.0F;
    return reader_.readI32(mode) && reader_.readFloat(engine_.volume) &&
           reader_.readI32(looping) && (looping == 0 || looping == 1) &&
           reader_.readI32(continuous) && (continuous == 0 || continuous == 1) &&
           reader_.readFloat(priority);
  }

  bool readCurves() {
    if (!reader_.readFloat(engine_.maxRpm)) {
      return false;
    }
    for (RealCurve& curve : engine_.volumeCurves) {
      initializeU32(kCurveSeed);
      if (!readCurve(curve)) {
        return false;
      }
    }
    for (RealCurve& curve : engine_.alphaCurves) {
      initializeU32(kCurveSeed);
      if (!readCurve(curve)) {
        return false;
      }
    }
    return true;
  }

  bool readCurve(RealCurve& curve) {
    while (true) {
      std::uint32_t chunkId = 0;
      if (!reader_.readU32(chunkId)) {
        return false;
      }
      switch (chunkId) {
      case kFacade:
        return curve.x.size() == curve.y.size() ||
               reader_.fail("sound-engine curve X/Y sizes differ");
      case 0x05002001U:
        if (!reader_.readFloatArray(curve.x)) {
          return false;
        }
        break;
      case 0x05002003U: {
        std::string ignored;
        if (!readId(ignored)) {
          return false;
        }
        break;
      }
      case 0x0501A000U:
        if (!reader_.readFloatArray(curve.y)) {
          return false;
        }
        break;
      case 0x0501A001U:
        if (!reader_.readFloatArray(curve.y) || !reader_.readI32(curve.interpolation)) {
          return false;
        }
        break;
      case 0x0501A002U:
        if (!reader_.readFloatArray(curve.y) || !reader_.readI32(curve.interpolation) ||
            !reader_.skip(16U)) {
          return false;
        }
        break;
      default:
        if (!skipSkippableChunk(chunkId)) {
          return false;
        }
        break;
      }
    }
  }

  bool skipSkippableChunk(const std::uint32_t chunkId) {
    std::uint32_t marker = 0;
    std::int32_t size = 0;
    if (!reader_.readU32(marker) || marker != kSkip ||
        !reader_.readCount(size, "skippable chunk")) {
      return reader_.fail("unknown unskippable sound-engine chunk " +
                          std::to_string(chunkId));
    }
    return reader_.skip(static_cast<std::size_t>(size));
  }

  Input& input_;
  Reader reader_;
  const DocumentProbe& document_;
  SoundEngine engine_;
  std::unordered_map<std::int32_t, std::string> externalPaths_;
  std::unordered_map<std::int32_t, std::size_t> componentNodes_;
  bool idVersionRead_ = false;
  std::unordered_map<std::uint32_t, std::string> ids_;
};

} // namespace

SoundEngineParseResult parseSoundEngine(Input& input) {
  const DocumentProbeResult probed = probeDocument(input, false);
  if (!probed) {
    SoundEngineParseResult result;
    result.error = probed.error;
    return result;
  }
  SoundEngineParser parser(input, probed.document);
  return parser.run();
}

SoundEngineComponentMix evaluateSoundEngineComponent(const SoundEngineComponent& component,
                                                     const float signedRpm) noexcept {
  SoundEngineComponentMix result;
  if (!std::isfinite(signedRpm)) {
    return result;
  }
  const bool reverse = signedRpm < 0.0F;
  const float rpm = reverse ? -signedRpm : signedRpm;
  const float direction = reverse ? -1.0F : 1.0F;
  const float fadeInStart = component.fadeInStartRpm * direction;
  const float fadeInEnd = component.fadeInEndRpm * direction;
  const float fadeOutStart = component.fadeOutStartRpm * direction;
  const float fadeOutEnd = component.fadeOutEndRpm * direction;
  if (rpm < fadeInStart) {
    result.volume = component.minVolume;
  } else if (rpm < fadeInEnd) {
    result.volume = std::lerp(component.minVolume, component.maxVolume,
                              (rpm - fadeInStart) / (fadeInEnd - fadeInStart));
  } else if (rpm < fadeOutStart) {
    result.volume = component.maxVolume;
  } else if (rpm < fadeOutEnd) {
    result.volume = std::lerp(component.maxVolume, component.minVolume,
                              (rpm - fadeOutStart) / (fadeOutEnd - fadeOutStart));
  } else {
    result.volume = component.minVolume;
  }

  const float pitchStart = component.pitchShiftStartRpm * direction;
  const float pitchEnd = component.pitchShiftEndRpm * direction;
  if (rpm < pitchStart) {
    result.pitch = component.minPitch;
  } else if (rpm < pitchEnd) {
    result.pitch = std::lerp(component.minPitch, component.maxPitch,
                             (rpm - pitchStart) / (pitchEnd - pitchStart));
  } else {
    result.pitch = component.maxPitch;
  }
  return result;
}

float evaluateSoundEngineVolume(const SoundEngine& engine, const float speed, const float distance,
                                const float signedRpm, const float acceleration) noexcept {
  const std::array inputs{speed, distance, signedRpm, acceleration};
  float result = 1.0F;
  for (std::size_t index = 0; index < inputs.size(); ++index) {
    result *= sampleCurve(engine.volumeCurves[index], inputs[index]);
  }
  return std::isfinite(result) ? result : 0.0F;
}

} // namespace tmnf::gbx
