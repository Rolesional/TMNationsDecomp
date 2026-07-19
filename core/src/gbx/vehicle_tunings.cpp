#include "tmnf/gbx/vehicle_tunings.hpp"

#include "tmnf/gbx/class_registry.hpp"
#include "tmnf/gbx/document.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace tmnf::gbx {
namespace {

constexpr std::uint32_t kFacade = 0xFACADE01U;
constexpr std::uint32_t kSkip = 0x534B4950U;
constexpr std::uint32_t kVehicleTuningsClassId = 0x0A030000U;
constexpr std::uint32_t kVehicleCarTuningClassId = 0x0A029000U;
constexpr std::uint32_t kVehicleTuningBaseChunk = 0x0A02E000U;
constexpr std::uint32_t kFuncKeysRealClassId = 0x0501A000U;
constexpr std::int32_t kMaximumCount = 1 << 20;

class Reader final {
public:
  explicit Reader(Input& input) : input_(input) {}

  bool readBytes(const std::span<std::byte> destination) {
    if (input_.position() > input_.size() ||
        destination.size() > input_.size() - input_.position()) {
      return fail("vehicle tuning read exceeds the declared GBX size");
    }
    if (!input_.readBytes(destination)) {
      const std::string_view detail = input_.error();
      return fail(detail.empty() ? "unable to read vehicle tuning GBX" : std::string(detail));
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
    return std::isfinite(value) || fail("vehicle tuning contains a non-finite float");
  }

  bool readBoolean(bool& value) {
    std::uint32_t encoded = 0;
    if (!readU32(encoded) || encoded > 1U) {
      return fail("vehicle tuning contains an invalid boolean");
    }
    value = encoded != 0U;
    return true;
  }

  bool readCount(std::int32_t& count, const char* const description) {
    if (!readI32(count) || count < 0 || count > kMaximumCount) {
      return fail(std::string("invalid vehicle tuning ") + description + " count");
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
    std::array<std::byte, 1024> buffer{};
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

class VehicleTuningsParser final {
public:
  VehicleTuningsParser(Input& input, const DocumentProbe& document)
      : input_(input), reader_(input), document_(document) {
    records_.emplace(0, document.header.classId);
    for (const ExternalNode& node : document.externalNodes) {
      records_.insert_or_assign(node.nodeIndex, 0U);
    }
  }

  VehicleTuningsParseResult run() {
    if (document_.header.classId != kVehicleTuningsClassId ||
        document_.firstChunkId != kVehicleTuningsClassId) {
      return failed("GBX document is not an uncompressed legacy vehicle-tunings file");
    }

    // probeDocument has consumed the first chunk ID, but no cipher boundary lies
    // between its body offset and this initializer.
    initializeU32(0x01001000U);
    std::int32_t version = 0;
    std::int32_t count = 0;
    if (!reader_.readI32(version) || version != 10 ||
        !reader_.readCount(count, "entry")) {
      return failedReader("unsupported vehicle tuning array version");
    }
    tunings_.entries.reserve(static_cast<std::size_t>(count));
    for (std::int32_t index = 0; index < count; ++index) {
      if (!readTuningReference()) {
        return failed(reader_.takeError());
      }
    }
    std::uint32_t facade = 0;
    if (!reader_.readI32(tunings_.activeIndex) || !reader_.readU32(facade) || facade != kFacade) {
      return failedReader("invalid vehicle tuning root terminator");
    }
    const VehicleTuning* const active = tunings_.active();
    if (active == nullptr || active->name.empty() || active->mass <= 0.0F ||
        active->maxSpeed <= 0.0F || active->reverseMaxSpeed <= 0.0F ||
        active->steerRadiusMin <= 0.0F || active->gravityCoef <= 0.0F ||
        active->gravityCoefAir <= 0.0F || active->m6GearRatio.empty() ||
        active->m6GearRatio.size() != active->m6MaxRpmByGear.size() ||
        active->m6GearRatio.size() != active->m6MinRpmByGear.size() ||
        active->m6GearRatio.size() != active->m6RpmWantedOnGearUp.size()) {
      return failed("active vehicle tuning is incomplete");
    }
    VehicleTuningsParseResult result;
    result.tunings = std::move(tunings_);
    return result;
  }

private:
  VehicleTuningsParseResult failed(std::string message) {
    VehicleTuningsParseResult result;
    result.error = std::move(message);
    if (result.error.empty()) {
      result.error = "unable to parse legacy vehicle tunings";
    }
    return result;
  }

  VehicleTuningsParseResult failedReader(const char* const fallback) {
    std::string error = reader_.takeError();
    return failed(error.empty() ? std::string(fallback) : std::move(error));
  }

  void initializeU32(const std::uint32_t value) {
    std::array<std::byte, 4> bytes{};
    for (std::size_t index = 0; index < bytes.size(); ++index) {
      bytes[index] = static_cast<std::byte>((value >> (index * 8U)) & 0xffU);
    }
    input_.initialize(bytes);
  }

  bool initializeNode(const std::uint32_t classId) {
    if (classId == kVehicleCarTuningClassId) {
      initializeU32(kVehicleTuningBaseChunk);
      return true;
    }
    const std::optional<std::uint32_t> seed = dummyWriteSeedClassId(classId);
    if (!seed.has_value()) {
      return reader_.fail("inline vehicle tuning node has no dummy-write seed");
    }
    initializeU32(*seed);
    return true;
  }

  bool initializeName(const std::string& name) {
    if (name.size() < 4U) {
      return reader_.fail("vehicle tuning name is too short for its dummy-write seed");
    }
    std::array<std::byte, 4> bytes{};
    for (std::size_t index = 0; index < bytes.size(); ++index) {
      bytes[index] = static_cast<std::byte>(static_cast<unsigned char>(name[index]));
    }
    input_.initialize(bytes);
    return true;
  }

  bool readId(std::string& value) {
    if (!idVersion_.has_value()) {
      std::int32_t version = 0;
      if (!reader_.readI32(version) || version != 3) {
        return reader_.fail("unsupported GBX ID version in vehicle tunings");
      }
      idVersion_ = version;
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
      const auto existing = ids_.find(encoded);
      if (existing == ids_.end()) {
        return reader_.fail("vehicle tuning references an unknown GBX ID");
      }
      value = existing->second;
      return true;
    }
    if (!reader_.readString(value)) {
      return false;
    }
    const std::uint32_t key = encoded + static_cast<std::uint32_t>(ids_.size()) + 1U;
    return ids_.emplace(key, value).second || reader_.fail("duplicate vehicle tuning GBX ID");
  }

  bool readTuningReference() {
    std::int32_t nodeIndex = -1;
    if (!reader_.readI32(nodeIndex) || nodeIndex < 0) {
      return reader_.fail("invalid vehicle tuning node reference");
    }
    if (const auto existing = tuningNodes_.find(nodeIndex); existing != tuningNodes_.end()) {
      tunings_.entries.push_back(tunings_.entries[existing->second]);
      return true;
    }
    if (records_.contains(nodeIndex)) {
      return reader_.fail("vehicle tuning node reference has the wrong type");
    }
    std::uint32_t classId = 0;
    if (!reader_.readU32(classId) || classId != kVehicleCarTuningClassId ||
        !initializeNode(classId)) {
      return reader_.fail("unsupported inline vehicle tuning class");
    }
    records_.emplace(nodeIndex, classId);
    const std::size_t entryIndex = tunings_.entries.size();
    tuningNodes_.emplace(nodeIndex, entryIndex);
    tunings_.entries.emplace_back();
    return readCarTuning(tunings_.entries.back());
  }

  bool readCurveReference(RealCurve* const output = nullptr) {
    std::int32_t nodeIndex = -1;
    if (!reader_.readI32(nodeIndex)) {
      return false;
    }
    if (nodeIndex == -1) {
      if (output != nullptr) {
        *output = {};
      }
      return true;
    }
    if (nodeIndex < 0) {
      return reader_.fail("invalid function-curve node reference");
    }
    if (const auto existing = curves_.find(nodeIndex); existing != curves_.end()) {
      if (output != nullptr) {
        *output = existing->second;
      }
      return true;
    }
    if (records_.contains(nodeIndex)) {
      return reader_.fail("function-curve node reference has the wrong type");
    }
    std::uint32_t classId = 0;
    if (!reader_.readU32(classId) || classId != kFuncKeysRealClassId ||
        !initializeNode(classId)) {
      return reader_.fail("unsupported vehicle tuning function-curve class");
    }
    records_.emplace(nodeIndex, classId);
    RealCurve curve;
    if (!readCurve(curve)) {
      return false;
    }
    const auto inserted = curves_.emplace(nodeIndex, std::move(curve));
    if (output != nullptr) {
      *output = inserted.first->second;
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
               reader_.fail("vehicle tuning curve X/Y sizes differ");
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
      return reader_.fail("unknown unskippable vehicle tuning chunk " +
                          std::to_string(chunkId));
    }
    return reader_.skip(static_cast<std::size_t>(size));
  }

  bool skipWords(const std::size_t count) { return reader_.skip(count * sizeof(std::uint32_t)); }

  bool readBaseChunk(VehicleTuning& tuning) {
    RealCurve ignored;
    float ignoredValue = 0.0F;
    if (!readId(tuning.name) || !readCurveReference(&ignored) ||
        !reader_.readFloat(ignoredValue)) {
      return false;
    }
    return initializeName(tuning.name);
  }

  bool readChunk000(VehicleTuning& tuning) {
    float ignored = 0.0F;
    return reader_.readFloat(ignored) && reader_.readFloat(tuning.steerRadiusMin) &&
           reader_.readFloat(tuning.steerRadiusCoef) && reader_.readFloat(tuning.maxSpeed) &&
           reader_.readFloat(tuning.suspensionSpring) &&
           reader_.readFloat(tuning.suspensionDamping) &&
           reader_.readFloat(tuning.suspensionMinimum) &&
           reader_.readFloat(tuning.suspensionMaximum) &&
           reader_.readFloat(tuning.suspensionRest) &&
           reader_.readFloat(tuning.relativeSpeedMultiplier) && skipWords(2U);
  }

  bool readChunk002(VehicleTuning& tuning) {
    return reader_.readFloat(tuning.mass) &&
           reader_.readFloat(tuning.centerOfMassAftForce) &&
           reader_.readFloat(tuning.centerOfMassDownUp);
  }

  bool readChunk00B(VehicleTuning& tuning) {
    return reader_.readFloat(tuning.inertiaMass) &&
           reader_.readFloat(tuning.inertiaHalfDiagonal[0]) &&
           reader_.readFloat(tuning.inertiaHalfDiagonal[1]) &&
           reader_.readFloat(tuning.inertiaHalfDiagonal[2]);
  }

  bool readChunk008(VehicleTuning& tuning) {
    return reader_.readFloat(tuning.reverseMaxSpeed) &&
           reader_.readFloat(tuning.turboBoost);
  }

  bool readChunk00A(VehicleTuning& tuning) {
    return reader_.readFloat(tuning.linearFluidFrictionCoef) &&
           reader_.readFloat(tuning.angularFluidFrictionCoef1);
  }

  bool readChunk018(VehicleTuning& tuning) {
    return reader_.readFloat(tuning.linearFluidFrictionCoef) &&
           reader_.readFloat(tuning.angularFluidFrictionCoef1) &&
           reader_.readFloat(tuning.angularFluidFrictionCoef2);
  }

  bool readChunk019(VehicleTuning& tuning) {
    return reader_.readFloat(tuning.bodyFrictionCoefConcrete) &&
           reader_.readFloat(tuning.bodyFrictionCoefMetal) &&
           reader_.readFloat(tuning.bodyRestCoefMetal) &&
           reader_.readFloat(tuning.bodyRestCoefConcrete) &&
           reader_.readFloat(tuning.wheelFrictionCoefConcrete) &&
           reader_.readFloat(tuning.wheelRestCoefConcrete) &&
           reader_.readFloat(tuning.wheelFrictionCoefMetal) &&
           reader_.readFloat(tuning.wheelRestCoefMetal);
  }

  bool readChunk02C(VehicleTuning& tuning) {
    float brakeCoef = 0.0F;
    float brakeMax = 0.0F;
    return readCurveReference() && reader_.readFloat(tuning.maxAngularSpeedYAirControl) &&
           reader_.readFloat(brakeCoef) && reader_.readFloat(brakeMax) &&
           reader_.readFloat(tuning.brakeMaxDynamic) &&
           reader_.readFloat(tuning.groundSlowDownCoef);
  }

  bool readChunk05D(VehicleTuning& tuning) {
    if (!reader_.readFloat(tuning.m6InertialMass) || !skipWords(1U) ||
        !readCurveReference(&tuning.reverseAcceleration) ||
        !readCurveReference() || !skipWords(13U) ||
        !reader_.readFloat(tuning.m6FrictionModulationWhenSlipAndBrake) || !skipWords(3U) ||
        !readCurveReference(&tuning.m6BurnoutRolloverFromSpeed) ||
        !readCurveReference(&tuning.m6DonutRolloverFromSpeed) || !skipWords(5U) ||
        !reader_.readFloat(tuning.m6MaxRpm) ||
        !reader_.readFloatArray(tuning.m6GearRatio) ||
        !reader_.readFloatArray(tuning.m6MaxRpmByGear) ||
        !reader_.readFloatArray(tuning.m6MinRpmByGear) || !skipWords(1U)) {
      return false;
    }
    return true;
  }

  bool readChunk060(VehicleTuning& tuning) {
    return reader_.readFloat(tuning.reverseMaxSpeed) &&
           reader_.readFloat(tuning.turboBoost) &&
           reader_.readFloat(tuning.turbo2Boost) &&
           reader_.readI32(tuning.turboDuration) &&
           reader_.readI32(tuning.turbo2Duration);
  }

  bool readCarTuning(VehicleTuning& tuning) {
    while (true) {
      std::uint32_t chunkId = 0;
      if (!reader_.readU32(chunkId)) {
        return false;
      }
      switch (chunkId) {
      case kFacade:
        return true;
      case kVehicleTuningBaseChunk:
        if (!readBaseChunk(tuning)) {
          return false;
        }
        break;
      case 0x0A02E001U: {
        bool ignored = false;
        if (!reader_.readBoolean(ignored)) {
          return false;
        }
        break;
      }
      case 0x0A029000U:
        if (!readChunk000(tuning)) {
          return false;
        }
        break;
      case 0x0A029001U: {
        std::string name;
        if (!readId(name)) {
          return false;
        }
        if (!name.empty()) {
          tuning.name = std::move(name);
        }
        break;
      }
      case 0x0A029002U:
        if (!readChunk002(tuning)) {
          return false;
        }
        break;
      case 0x0A029004U:
        if (!reader_.readBoolean(tuning.fakeEngine)) {
          return false;
        }
        break;
      case 0x0A029005U:
        if (!reader_.readFloat(tuning.gravityCoef)) return false;
        break;
      case 0x0A029006U:
        if (!skipWords(4U)) return false;
        break;
      case 0x0A029007U:
        if (!reader_.readFloat(tuning.steerSpeed)) return false;
        break;
      case 0x0A029008U:
        if (!readChunk008(tuning)) return false;
        break;
      case 0x0A029009U:
        if (!skipWords(1U)) return false;
        break;
      case 0x0A02900AU:
        if (!readChunk00A(tuning)) return false;
        break;
      case 0x0A02900BU:
        if (!readChunk00B(tuning)) return false;
        break;
      case 0x0A02900CU:
        if (!skipWords(1U)) return false;
        break;
      case 0x0A02900DU:
        if (!reader_.readI32(tuning.tireMaterial)) return false;
        break;
      case 0x0A02900EU:
        if (!reader_.readI32(tuning.shockModel)) return false;
        break;
      case 0x0A029010U:
        if (!reader_.readI32(tuning.steerModel) ||
            !reader_.readFloat(tuning.steerGroundTorque)) return false;
        break;
      case 0x0A029011U:
        if (!skipWords(9U)) return false;
        break;
      case 0x0A029012U:
        if (!skipWords(4U)) return false;
        break;
      case 0x0A029013U:
        if (!skipWords(12U)) return false;
        break;
      case 0x0A029014U:
        if (!skipWords(2U)) return false;
        break;
      case 0x0A029015U:
        if (!skipWords(5U)) return false;
        break;
      case 0x0A029016U:
        if (!skipWords(2U)) return false;
        break;
      case 0x0A029017U:
        if (!skipWords(8U)) return false;
        break;
      case 0x0A029018U:
        if (!readChunk018(tuning)) return false;
        break;
      case 0x0A029019U:
        if (!readChunk019(tuning)) return false;
        break;
      case 0x0A02901AU:
      case 0x0A02901BU:
        if (!skipWords(1U)) return false;
        break;
      case 0x0A02901DU:
        if (!reader_.readFloat(tuning.gravityCoefAir) || !skipWords(1U)) return false;
        break;
      case 0x0A02901EU:
        if (!reader_.readI32(tuning.airControlDuration)) return false;
        break;
      case 0x0A02901FU:
        if (!skipWords(3U)) return false;
        break;
      case 0x0A029020U:
        if (!reader_.readFloat(tuning.steerAngleMax) ||
            !reader_.readFloat(tuning.slipAngleForceMax) ||
            !reader_.readFloat(tuning.slipAngleForceCoef1) ||
            !reader_.readFloat(tuning.slipAngleForceCoef2)) return false;
        break;
      case 0x0A029021U:
        if (!reader_.readFloat(tuning.slipAngleForceMax)) return false;
        break;
      case 0x0A029022U:
        if (!skipWords(2U)) return false;
        break;
      case 0x0A029023U:
        if (!reader_.readFloat(tuning.angularSpeedYImpulseScale) ||
            !reader_.readFloat(tuning.steerLowSpeed)) return false;
        break;
      case 0x0A029024U:
        if (!readCurveReference(&tuning.acceleration)) return false;
        break;
      case 0x0A029026U:
        if (!reader_.readFloat(tuning.lateralSlopeAdherenceMin) ||
            !reader_.readFloat(tuning.lateralSlopeAdherenceMax) ||
            !reader_.readFloat(tuning.axialSlopeAdherenceMin) ||
            !reader_.readFloat(tuning.axialSlopeAdherenceMax)) return false;
        break;
      case 0x0A029027U:
        if (!reader_.readFloat(tuning.steerGroundTorqueSlippingCoef) ||
            !reader_.readFloat(tuning.maxSideFrictionBlendCoef)) return false;
        break;
      case 0x0A029028U:
        if (!skipWords(1U) || !readCurveReference(&tuning.maxSideFriction) ||
            !reader_.readFloat(tuning.maxSideFrictionSliding) || !skipWords(2U) ||
            !reader_.readFloat(tuning.absorbTension)) return false;
        break;
      case 0x0A029029U:
        if (!reader_.readFloat(tuning.sideFriction1) ||
            !readCurveReference(&tuning.rolloverLateral)) return false;
        break;
      case 0x0A02902AU:
        if (!readCurveReference()) return false;
        break;
      case 0x0A02902BU:
        if (!readCurveReference(&tuning.steerSlowDown) || !skipWords(3U)) return false;
        break;
      case 0x0A02902CU:
        if (!readChunk02C(tuning)) return false;
        break;
      case 0x0A02902DU:
        if (!skipWords(1U)) return false;
        break;
      case 0x0A02902EU:
        if (!reader_.readFloat(tuning.steerSlowDownCoef) || !skipWords(1U)) return false;
        break;
      case 0x0A02902FU:
        if (!reader_.readI32(tuning.turboDuration)) return false;
        break;
      case 0x0A029030U:
        if (!readCurveReference(&tuning.steerDriveTorque)) return false;
        break;
      case 0x0A029031U:
        if (!skipWords(2U)) return false;
        break;
      case 0x0A029032U:
        if (!skipWords(8U)) return false;
        break;
      case 0x0A029033U:
        if (!reader_.readFloat(tuning.angularSpeedClamp)) return false;
        break;
      case 0x0A029034U:
        if (!reader_.readFloat(tuning.linearSpeed2PositiveDeltaMax)) return false;
        break;
      case 0x0A029035U:
        if (!skipWords(1U)) return false;
        break;
      case 0x0A029036U:
        if (!skipWords(3U) || !readCurveReference()) return false;
        break;
      case 0x0A029037U:
        if (!readCurveReference()) return false;
        break;
      case 0x0A029038U:
        if (!skipWords(1U) || !readCurveReference() || !skipWords(2U)) return false;
        break;
      case 0x0A029039U:
      case 0x0A02903AU:
        if (!skipWords(2U)) return false;
        break;
      case 0x0A02903BU:
      case 0x0A02903CU:
        if (!skipWords(1U)) return false;
        break;
      case 0x0A02903DU:
        if (!readCurveReference()) return false;
        break;
      case 0x0A02903EU:
        if (!skipWords(1U)) return false;
        break;
      case 0x0A02903FU:
      case 0x0A029040U:
        if (!readCurveReference()) return false;
        break;
      case 0x0A029041U:
        if (!skipWords(2U)) return false;
        break;
      case 0x0A029042U:
      case 0x0A029043U:
      case 0x0A029044U:
        if (!skipWords(1U)) return false;
        break;
      case 0x0A029045U:
        if (!skipWords(5U)) return false;
        break;
      case 0x0A029046U:
        if (!skipWords(3U) || !readCurveReference() || !readCurveReference() ||
            !readCurveReference()) return false;
        break;
      case 0x0A029047U:
        if (!skipWords(1U)) return false;
        break;
      case 0x0A029048U:
      case 0x0A029049U:
        if (!readCurveReference(&tuning.modulationFromWheelCompression)) return false;
        break;
      case 0x0A02904AU:
        if (!skipWords(1U) || !reader_.readFloat(tuning.m6InertialMass)) return false;
        break;
      case 0x0A02904DU:
        if (!reader_.readFloat(tuning.m6ForceEpsilon)) return false;
        break;
      case 0x0A02904EU:
        if (!reader_.readFloat(tuning.m6InertialTorqueModulationX) ||
            !reader_.readFloat(tuning.m6InertialTorqueModulationZ)) return false;
        break;
      case 0x0A02904FU:
        if (!skipWords(1U) || !readCurveReference()) return false;
        break;
      case 0x0A029051U:
        if (!skipWords(1U)) return false;
        break;
      case 0x0A029052U:
        if (!readCurveReference(&tuning.m6RolloverLateralFromSpeedRatio)) return false;
        break;
      case 0x0A029053U:
        if (!skipWords(1U)) return false;
        break;
      case 0x0A029056U:
        if (!reader_.readFloat(tuning.m6BurnoutRpmAcceleration) ||
            !reader_.readFloat(tuning.m6AirRpmAcceleration) ||
            !reader_.readFloat(tuning.m6AirRpmDeadening)) return false;
        break;
      case 0x0A029057U:
        if (!reader_.readFloatArray(tuning.m6RpmWantedOnGearUp)) return false;
        break;
      case 0x0A029058U:
        if (!reader_.readFloat(tuning.m6RpmLossOnGearUp)) return false;
        break;
      case 0x0A029059U: {
        if (!reader_.readFloat(tuning.m6RpmGainOnGearDown) ||
            !reader_.readFloat(tuning.m6SpeedLimitPositiveForTakeOffFront) ||
            !reader_.readFloat(tuning.m6RpmGainOnTakeOff) ||
            !reader_.readFloat(tuning.m6RpmLossOnTakeOffFinished)) return false;
        break;
      }
      case 0x0A02905AU:
        if (!reader_.readFloat(tuning.m6SpeedLimitPositiveForTakeOffRear) ||
            !reader_.readFloat(tuning.m6SpeedLimitNegativeForTakeOffFront) ||
            !reader_.readFloat(tuning.m6SpeedLimitNegativeForTakeOffRear)) return false;
        break;
      case 0x0A02905BU:
        if (!readCurveReference() || !skipWords(1U)) return false;
        break;
      case 0x0A02905CU:
        if (!skipWords(2U)) return false;
        break;
      case 0x0A02905DU:
        if (!readChunk05D(tuning)) return false;
        break;
      case 0x0A02905EU:
        if (!readCurveReference(&tuning.airControlZCoefFromAngularSpeed)) return false;
        break;
      case 0x0A02905FU:
        if (!skipWords(1U)) return false;
        break;
      case 0x0A029060U:
        if (!readChunk060(tuning)) return false;
        break;
      case 0x0A029061U:
        if (!readCurveReference(&tuning.visualSteerAngleFromSpeed)) return false;
        break;
      case 0x0A029062U:
      case 0x0A029063U:
      case 0x0A029064U:
      case 0x0A029065U:
      case 0x0A029066U:
        if (!skipWords(1U)) return false;
        break;
      default:
        if (!skipSkippableChunk(chunkId)) {
          return false;
        }
        break;
      }
    }
  }

  Input& input_;
  Reader reader_;
  const DocumentProbe& document_;
  VehicleTunings tunings_;
  std::unordered_map<std::int32_t, std::uint32_t> records_;
  std::unordered_map<std::int32_t, std::size_t> tuningNodes_;
  std::unordered_map<std::int32_t, RealCurve> curves_;
  std::optional<std::int32_t> idVersion_;
  std::unordered_map<std::uint32_t, std::string> ids_;
};

} // namespace

VehicleTuningsParseResult parseVehicleTunings(Input& input) {
  const DocumentProbeResult probed = probeDocument(input, false);
  if (!probed) {
    VehicleTuningsParseResult result;
    result.error = probed.error;
    return result;
  }
  VehicleTuningsParser parser(input, probed.document);
  return parser.run();
}

} // namespace tmnf::gbx
