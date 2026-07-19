#include "tmnf/legacy/archive.hpp"
#include "tmnf/legacy/application.hpp"
#include "tmnf/legacy/array.hpp"
#include "tmnf/legacy/intrusive_ptr.hpp"
#include "tmnf/legacy/localization.hpp"
#include "tmnf/legacy/math.hpp"
#include "tmnf/legacy/string_view.hpp"

#include <array>
#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <string>
#include <string_view>

namespace {

struct ArchiveProbe {
  static inline int constructions;
  static inline int destructions;
  static inline bool throwOnWrite;
  static inline void* node;
  static inline std::uint32_t first;
  static inline std::uint32_t second;
  static inline std::uint32_t third;
  static inline std::uint32_t fourth;

  ArchiveProbe() { ++constructions; }
  ~ArchiveProbe() { ++destructions; }

  int writeNode(
      int* const value,
      const std::uint32_t flags,
      const std::uint32_t mode,
      const bool transactional) {
    node = value;
    first = flags;
    second = mode;
    third = transactional ? 1U : 0U;
    if (throwOnWrite) {
      throw 1;
    }
    return 17;
  }

  int readNode(
      const std::uint32_t a,
      int* const b,
      const std::uint32_t c,
      const std::uint32_t d) {
    first = a;
    node = b;
    third = c;
    fourth = d;
    return 23;
  }

  static void reset() {
    constructions = 0;
    destructions = 0;
    throwOnWrite = false;
    node = nullptr;
    first = second = third = fourth = 0;
  }
};

struct ApplicationProbe {
  int argumentCalls{};
  int stopCalls{};
  const char* lastArgument{};

  bool handleGameArgument(const char* const argument) {
    ++argumentCalls;
    lastArgument = argument;
    return true;
  }

  void stopApplication() { ++stopCalls; }
};

struct RefProbe {
  static inline int sequence;
  int refs{1};
  int addOrder{};
  int releaseOrder{};

  void addRef() {
    addOrder = ++sequence;
    ++refs;
  }

  void release() {
    releaseOrder = ++sequence;
    --refs;
  }
};

struct StopProbe {
  std::array<int, 8>* events{};
  std::size_t* eventCount{};
  bool extension{};
  std::uint32_t state{73U};
  int refs{1};

  void record(const int event) { (*events)[(*eventCount)++] = event; }
  [[nodiscard]] bool hasExtension() const { return extension; }
  void destroyExtension() {
    record(3);
    extension = false;
  }
  [[nodiscard]] std::uint32_t stopState() {
    record(4);
    return state;
  }
  void addRef() {
    record(5);
    ++refs;
  }
  void stopApplication() { record(6); }
  void release() {
    record(7);
    --refs;
  }
};

struct FastCallbackProbe {
  int calls{};
  int lastValue{};

  int onNat(const std::array<int, 2>& value) {
    ++calls;
    return lastValue = value[0] + value[1];
  }

  std::size_t onString(const std::string& value) {
    ++calls;
    lastValue = static_cast<int>(value.size());
    return value.size();
  }
};

using DecoderBytes = std::array<unsigned char, 8>;

bool decodeMatches(
    DecoderBytes bytes,
    const std::uint32_t expectedValue,
    const std::size_t expectedConsumed,
    const std::size_t changedIndex = 8U,
    const unsigned char changedValue = 0U) {
  auto* cursor = bytes.data();
  const auto value = tmnf::legacy::decodeLegacyUtf8CodePoint(cursor);
  return value == expectedValue &&
         static_cast<std::size_t>(cursor - bytes.data()) == expectedConsumed &&
         (changedIndex == bytes.size() || bytes[changedIndex] == changedValue);
}

} // namespace

int main() {
  const auto empty = tmnf::legacy::nullableStringView<char>(nullptr);
  const auto narrow = tmnf::legacy::nullableStringView("TrackMania");
  const auto wide = tmnf::legacy::nullableStringView(u"Nations");
  const bool nullableConstructionMatches =
      tmnf::legacy::makeNullableString(nullptr).empty() &&
      tmnf::legacy::makeNullableString("Forever") == "Forever";
  const bool nullableWideConstructionMatches =
      tmnf::legacy::makeNullableWideString(nullptr).empty() &&
      tmnf::legacy::makeNullableWideString(u"Forever") == u"Forever";
  const bool legacyUtf8DecoderMatches =
      decodeMatches(DecoderBytes{0U}, 0U, 1U) &&
      decodeMatches(DecoderBytes{'A', 0U}, 'A', 1U) &&
      decodeMatches(DecoderBytes{0x80U, 'A', 0U}, 'A', 2U) &&
      decodeMatches(DecoderBytes{0xc2U, 0x80U, 0U}, 0x80U, 2U) &&
      decodeMatches(DecoderBytes{0xc2U, 0x0aU, 0U}, 0x0aU, 2U, 1U, 0x0aU) &&
      decodeMatches(DecoderBytes{0xc2U, 0x09U, 0U}, 0x09U, 2U, 1U, 0x09U) &&
      decodeMatches(DecoderBytes{0xc2U, 0x20U, 0U}, 0x20U, 2U, 1U, 0x20U) &&
      decodeMatches(DecoderBytes{0xc2U, 0x3aU, 0U}, 0x3aU, 2U, 1U, 0x3aU) &&
      decodeMatches(DecoderBytes{0xc2U, 0x7bU, 0U}, 0x7bU, 2U, 1U, 0x7bU) &&
      decodeMatches(DecoderBytes{0xc2U, 'A', 'B', 0U}, 'B', 3U, 1U, 0x81U) &&
      decodeMatches(DecoderBytes{0xe0U, 0xa0U, 0x80U, 0U}, 0x800U, 3U) &&
      decodeMatches(DecoderBytes{0xe1U, 0x80U, 0x80U, 0U}, 0x1000U, 3U) &&
      decodeMatches(DecoderBytes{0xefU, 0xbbU, 0xbfU, 'A', 0U}, 'A', 4U) &&
      decodeMatches(DecoderBytes{0xe1U, 'A', 'B', 0U}, 'A', 2U) &&
      decodeMatches(DecoderBytes{0xe1U, 0x80U, 'A', 0U}, 'A', 3U) &&
      decodeMatches(DecoderBytes{0xe0U, 0x9fU, 0x80U, 'A', 0U}, 'A', 4U) &&
      decodeMatches(DecoderBytes{0xf0U, 0x90U, 0x80U, 0x80U, 0U}, 0x10000U, 4U) &&
      decodeMatches(DecoderBytes{0xf1U, 0x80U, 0x80U, 0x80U, 0U}, 0x40000U, 4U) &&
      decodeMatches(DecoderBytes{0xf1U, 'A', 0x80U, 0x80U, 0U}, 'A', 2U) &&
      decodeMatches(DecoderBytes{0xf1U, 0x80U, 'A', 0x80U, 0U}, 'A', 3U) &&
      decodeMatches(DecoderBytes{0xf1U, 0x80U, 0x80U, 'A', 0U}, 'A', 4U) &&
      decodeMatches(DecoderBytes{0xf0U, 0x8fU, 0x80U, 0x80U, 'A', 0U}, 'A', 5U) &&
      decodeMatches(DecoderBytes{0xf8U, 0x88U, 0x80U, 0x80U, 0x80U, 0U}, 0x200000U, 5U) &&
      decodeMatches(DecoderBytes{0xf9U, 0x80U, 0x80U, 0x80U, 0x80U, 0U}, 0x1000000U, 5U) &&
      decodeMatches(DecoderBytes{0xf9U, 'A', 0x80U, 0x80U, 0x80U, 0U}, 'A', 2U) &&
      decodeMatches(DecoderBytes{0xf9U, 0x80U, 'A', 0x80U, 0x80U, 0U}, 'A', 3U) &&
      decodeMatches(DecoderBytes{0xf9U, 0x80U, 0x80U, 'A', 0x80U, 0U}, 'A', 4U) &&
      decodeMatches(DecoderBytes{0xf9U, 0x80U, 0x80U, 0x80U, 'A', 0U}, 'A', 5U) &&
      decodeMatches(DecoderBytes{0xf8U, 0x87U, 0x80U, 0x80U, 0x80U, 'A', 0U}, 'A', 6U) &&
      decodeMatches(DecoderBytes{0xfcU, 0x84U, 0x80U, 0x80U, 0x80U, 0x80U, 0U}, 0x4000000U, 6U) &&
      decodeMatches(DecoderBytes{0xfdU, 0x80U, 0x80U, 0x80U, 0x80U, 0x80U, 0U}, 0x40000000U, 6U) &&
      decodeMatches(DecoderBytes{0xfdU, 'A', 0x80U, 0x80U, 0x80U, 0x80U, 0U}, 'A', 2U) &&
      decodeMatches(DecoderBytes{0xfdU, 0x80U, 'A', 0x80U, 0x80U, 0x80U, 0U}, 'A', 3U) &&
      decodeMatches(DecoderBytes{0xfdU, 0x80U, 0x80U, 'A', 0x80U, 0x80U, 0U}, 'A', 4U) &&
      decodeMatches(DecoderBytes{0xfdU, 0x80U, 0x80U, 0x80U, 'A', 0x80U, 0U}, 'A', 5U) &&
      decodeMatches(DecoderBytes{0xfdU, 0x80U, 0x80U, 0x80U, 0x80U, 'A', 0U}, 'A', 6U) &&
      decodeMatches(DecoderBytes{0xfcU, 0x83U, 0x80U, 0x80U, 0x80U, 0x80U, 'A', 0U}, 'A', 7U) &&
      decodeMatches(DecoderBytes{0xfeU, 'A', 0U}, 'A', 2U);
  std::array<char, 4> widenedSource{'A', static_cast<char>(0xff), '\0', 'B'};
  const auto directlyWidened = tmnf::legacy::makeWideFastString(
      {3U, widenedSource.data()});
  std::array<char, 14> bomSource{
      static_cast<char>(0xef), static_cast<char>(0xbb), static_cast<char>(0xbf), 'A',
      static_cast<char>(0xc2), static_cast<char>(0xa2), static_cast<char>(0xe2),
      static_cast<char>(0x82), static_cast<char>(0xac), static_cast<char>(0xf0),
      static_cast<char>(0x9f), static_cast<char>(0x98), static_cast<char>(0x80), '\0'};
  const auto bomWidened = tmnf::legacy::makeWideFastString({13U, bomSource.data()});
  std::u16string assignedBom = u"stale";
  tmnf::legacy::assignUtf8BomWide(assignedBom, {13U, bomSource.data()});
  const bool wideConversionMatches =
      directlyWidened == std::u16string{u'A', u'\u00ff', u'\0'} &&
      bomWidened == std::u16string{u'A', u'\u00a2', u'\u20ac', u'\uf600'} &&
      assignedBom == bomWidened &&
      tmnf::legacy::makeWideFastString({0U, nullptr}).empty();
  std::string assigned = "Nations";
  tmnf::legacy::assignView(assigned, {assigned.data(), 3U});
  const bool sameBufferWasPreserved = assigned == "Nations";
  tmnf::legacy::assignView(assigned, std::string_view{"A\0B", 3U});
  const bool explicitLengthWasCopied = assigned.size() == 3U && assigned[1] == '\0';
  tmnf::legacy::assignNullable(assigned, nullptr);
  std::u16string wideAssigned = u"Nations";
  tmnf::legacy::StringStorageView<char16_t> sameWideStorage{
      3U, wideAssigned.data()};
  tmnf::legacy::assignStorage(wideAssigned, sameWideStorage);
  const bool sameWideBufferWasPreserved = wideAssigned == u"Nations";
  constexpr char16_t embeddedWide[] = {u'A', u'\0', u'B', u'\0'};
  tmnf::legacy::assignStorage(
      wideAssigned, tmnf::legacy::StringStorageView<char16_t>{3U, embeddedWide});
  const bool explicitWideLengthWasCopied =
      wideAssigned.size() == 3U && wideAssigned[1] == u'\0';
  char clearedBuffer[] = "Nations";
  std::uint32_t clearedLength = 7U;
  tmnf::legacy::clearBuffer(clearedLength, clearedBuffer);
  char untouchedBuffer[] = "Track";
  std::uint32_t untouchedLength = 0U;
  tmnf::legacy::clearBuffer(untouchedLength, untouchedBuffer);
  const bool bufferClearMatches = clearedLength == 0U && clearedBuffer[0] == '\0' &&
                                  untouchedLength == 0U && untouchedBuffer[0] == 'T';
  char emptyNarrow[] = "";
  char initializedBacking[] = "used";
  char* initializedData = initializedBacking;
  std::uint32_t initializedLength = 4U;
  tmnf::legacy::initializeStringStorage(
      initializedLength, initializedData, emptyNarrow);
  const bool initializedStorageMatches =
      initializedLength == 0U && initializedData == emptyNarrow;
  std::array<unsigned char, 8> narrowAllocation{};
  char* narrowData = reinterpret_cast<char*>(narrowAllocation.data() + 4);
  narrowData[-1] = static_cast<char>(0x80U);
  std::uint32_t narrowLength = 3U;
  void* freedStorage = nullptr;
  const auto captureFree = [&freedStorage](void* value) { freedStorage = value; };
  tmnf::legacy::releaseStringStorage(
      narrowLength, narrowData, emptyNarrow, captureFree);
  const bool narrowStorageMatches = narrowLength == 0U && narrowData == emptyNarrow &&
                                    freedStorage == narrowAllocation.data();
  std::array<unsigned char, 8> shortNarrowAllocation{};
  narrowData = reinterpret_cast<char*>(shortNarrowAllocation.data() + 1);
  narrowData[-1] = 0;
  narrowLength = 1U;
  tmnf::legacy::releaseStringStorage(
      narrowLength, narrowData, emptyNarrow, captureFree);
  const bool shortNarrowStorageMatches = freedStorage == shortNarrowAllocation.data();
  freedStorage = nullptr;
  tmnf::legacy::releaseStringStorage(
      narrowLength, narrowData, emptyNarrow, captureFree);
  const bool sentinelStorageMatches = freedStorage == nullptr;
  alignas(char16_t) std::array<unsigned char, 8> wideAllocation{};
  auto* wideData = reinterpret_cast<char16_t*>(wideAllocation.data() + sizeof(char16_t));
  reinterpret_cast<unsigned char*>(wideData)[-1] = 0U;
  char16_t emptyWide[] = u"";
  char16_t initializedWideBacking[] = u"used";
  char16_t* initializedWideData = initializedWideBacking;
  std::uint32_t initializedWideLength = 4U;
  tmnf::legacy::initializeStringStorage(
      initializedWideLength, initializedWideData, emptyWide);
  const bool initializedWideStorageMatches =
      initializedWideLength == 0U && initializedWideData == emptyWide;
  std::uint32_t wideLength = 2U;
  tmnf::legacy::releaseStringStorage(
      wideLength, wideData, emptyWide, captureFree);
  const bool wideStorageMatches = wideLength == 0U && wideData == emptyWide &&
                                  freedStorage == wideAllocation.data();
  alignas(char16_t) std::array<unsigned char, 8> longWideAllocation{};
  wideData = reinterpret_cast<char16_t*>(longWideAllocation.data() + 4);
  reinterpret_cast<unsigned char*>(wideData)[-1] = 0x80U;
  wideLength = 2U;
  tmnf::legacy::releaseStringStorage(
      wideLength, wideData, emptyWide, captureFree);
  const bool longWideStorageMatches = freedStorage == longWideAllocation.data();
  freedStorage = nullptr;
  tmnf::legacy::releaseStringStorage(
      wideLength, wideData, emptyWide, captureFree);
  const bool wideSentinelStorageMatches = freedStorage == nullptr;

  std::uint32_t hashLength = 0;
  const bool hashMatches =
      tmnf::legacy::gettextHash(u"|Menu|Play", &hashLength) == 0x05d98149U &&
      hashLength == 10U && tmnf::legacy::gettextHash(u"\uffff\uffff\uffff\uffff") == 0x0110eeffU;
  constexpr char16_t contextual[] = u"|Menu|Play";
  constexpr char16_t incompleteContext[] = u"|Menu";
  constexpr char16_t plain[] = u"Play";
  const bool contextMatches =
      tmnf::legacy::stripTranslationContext(contextual) == contextual + 6 &&
      tmnf::legacy::stripTranslationContext(incompleteContext) == incompleteContext &&
      tmnf::legacy::stripTranslationContext(plain) == plain;

  std::array<const char16_t*, 3> sources{u"First collision", u"Second collision", u"Play"};
  std::array<const char16_t*, 3> translations{u"Unused", u"Unused", u"|Verb|Jouer"};
  std::array<std::uint8_t, 3> used{};
  constexpr std::array<std::uint16_t, 5> slots{2U, 0U, 0xffffU, 1U, 0xffffU};
  tmnf::legacy::TranslationCatalog catalog{sources, translations, used, 0U, slots};
  const auto found = tmnf::legacy::findTranslation(catalog, u"Play");
  const auto foundAgain = tmnf::legacy::findTranslation(catalog, u"Play");
  const auto missing = tmnf::legacy::findTranslation(catalog, u"Missing");
  const bool lookupMatches = found == 2U && foundAgain == 2U &&
                             missing == tmnf::legacy::kTranslationNotFound &&
                             used[2] == 1U && catalog.usedCount == 1U;
  const bool resolutionMatches =
      std::u16string_view{tmnf::legacy::resolveTranslation(catalog, u"Play")} == u"Jouer" &&
      std::u16string_view{tmnf::legacy::resolveTranslation(catalog, u"|Menu|Missing")} ==
          u"Missing";
  tmnf::legacy::TranslationCatalog catalogWithoutHash{sources, translations, used};
  constexpr char16_t emptyText[] = u"";
  const bool fallbackMatches =
      tmnf::legacy::resolveTranslation(catalogWithoutHash, contextual) == contextual + 6 &&
      tmnf::legacy::resolveTranslation(catalog, emptyText) == emptyText;
  auto& globalCatalog = tmnf::legacy::translationCatalog();
  globalCatalog = catalog;
  const bool wrapperMatches = std::u16string_view{tmnf::legacy::translate(u"Play")} == u"Jouer";
  globalCatalog = {};

  int archiveNode = 0;
  ArchiveProbe::reset();
  const auto writeResult =
      tmnf::legacy::writeArchiveNode<ArchiveProbe>(&archiveNode, 10U, true);
  const bool writeWrapperMatches =
      writeResult == 17 && ArchiveProbe::constructions == 1 &&
      ArchiveProbe::destructions == 1 && ArchiveProbe::node == &archiveNode &&
      ArchiveProbe::first == 10U && ArchiveProbe::second == 7U &&
      ArchiveProbe::third == 1U;
  ArchiveProbe::reset();
  ArchiveProbe::throwOnWrite = true;
  bool writeExceptionObserved = false;
  try {
    static_cast<void>(
        tmnf::legacy::writeArchiveNode<ArchiveProbe>(&archiveNode, 8U, false));
  } catch (int) {
    writeExceptionObserved = true;
  }
  const bool writeUnwindMatches = writeExceptionObserved &&
                                  ArchiveProbe::constructions == 1 &&
                                  ArchiveProbe::destructions == 1;
  ArchiveProbe::reset();
  const auto readResult =
      tmnf::legacy::readArchiveNode<ArchiveProbe>(1U, &archiveNode, 3U, 4U);
  const bool readWrapperMatches =
      readResult == 23 && ArchiveProbe::constructions == 1 &&
      ArchiveProbe::destructions == 1 && ArchiveProbe::first == 1U &&
      ArchiveProbe::node == &archiveNode && ArchiveProbe::third == 3U &&
      ArchiveProbe::fourth == 4U;

  ApplicationProbe application;
  const bool argumentWrapperMatches =
      !tmnf::legacy::handleGameArgument<ApplicationProbe>(nullptr, "ignored") &&
      tmnf::legacy::handleGameArgument(&application, "/bench") &&
      application.argumentCalls == 1 &&
      std::string_view{application.lastArgument} == "/bench";
  int fallbackStops = 0;
  tmnf::legacy::stopGameApplication(&application, [&fallbackStops] { ++fallbackStops; });
  tmnf::legacy::stopGameApplication<ApplicationProbe>(
      nullptr, [&fallbackStops] { ++fallbackStops; });
  const bool stopWrapperMatches = application.stopCalls == 1 && fallbackStops == 1;
  std::u16string pendingArgument;
  const bool pendingArgumentMatches =
      !tmnf::legacy::setPendingGameArgument(nullptr, u"ignored") &&
      tmnf::legacy::setPendingGameArgument(&pendingArgument, u"first") &&
      !tmnf::legacy::setPendingGameArgument(&pendingArgument, u"second") &&
      pendingArgument == u"first";
  std::array<int, 8> stopEvents{};
  std::size_t stopEventCount = 0;
  StopProbe stopProbe{&stopEvents, &stopEventCount, true};
  StopProbe* stopApplication = &stopProbe;
  char diagnostic[] = "pending";
  std::uint32_t diagnosticLength = 7U;
  std::uint32_t preservedState = 0U;
  tmnf::legacy::stopGbxGame(
      stopApplication,
      diagnosticLength,
      diagnostic,
      preservedState,
      [&] { stopEvents[stopEventCount++] = 1; },
      [&] { stopEvents[stopEventCount++] = 2; });
  const std::array expectedStopEvents{1, 2, 3, 4, 5, 6, 7, 0};
  const bool fullStopMatches = stopApplication == nullptr && diagnosticLength == 0U &&
                               diagnostic[0] == '\0' && preservedState == 73U &&
                               stopProbe.refs == 1 && !stopProbe.extension &&
                               stopEventCount == 7U && stopEvents == expectedStopEvents;
  stopEvents.fill(0);
  stopEventCount = 0;
  StopProbe noExtensionProbe{&stopEvents, &stopEventCount, false};
  stopApplication = &noExtensionProbe;
  diagnostic[0] = 'x';
  preservedState = 0U;
  tmnf::legacy::stopGbxGame(
      stopApplication,
      diagnosticLength,
      diagnostic,
      preservedState,
      [&] { stopEvents[stopEventCount++] = 1; },
      [&] { stopEvents[stopEventCount++] = 2; });
  const std::array expectedNoExtensionEvents{1, 2, 4, 5, 6, 7, 0, 0};
  const bool noExtensionStopMatches =
      stopApplication == nullptr && diagnostic[0] == 'x' && preservedState == 73U &&
      noExtensionProbe.refs == 1 && stopEventCount == 6U &&
      stopEvents == expectedNoExtensionEvents;
  stopEventCount = 0;
  tmnf::legacy::stopGbxGame(
      stopApplication,
      diagnosticLength,
      diagnostic,
      preservedState,
      [&] { stopEvents[stopEventCount++] = 1; },
      [&] { stopEvents[stopEventCount++] = 2; });
  const bool emptyStopMatches = diagnostic[0] == 'x' && stopEventCount == 2U &&
                                stopEvents[0] == 1 && stopEvents[1] == 2;
  const bool slot94PredicateMatches =
      !tmnf::legacy::gbxGameSlot94Predicate(0U, 0U, 0U, 0U, 0U) &&
      tmnf::legacy::gbxGameSlot94Predicate(1U, 0U, 0U, 0U, 0U) &&
      tmnf::legacy::gbxGameSlot94Predicate(0U, 1U, 0U, 0U, 0U) &&
      !tmnf::legacy::gbxGameSlot94Predicate(0U, 1U, 1U, 0U, 0U) &&
      !tmnf::legacy::gbxGameSlot94Predicate(0U, 1U, 0U, 1U, 0U) &&
      tmnf::legacy::gbxGameSlot94Predicate(0U, 0U, 0U, 0U, 1U);
  FastCallbackProbe callbackProbe;
  const std::array natArgument{2, 5};
  const std::string stringArgument = "Forever";
  const auto natCallbackResult = tmnf::legacy::invokeFastCallbackInstance1P(
      &callbackProbe, &FastCallbackProbe::onNat, natArgument);
  const auto stringCallbackResult = tmnf::legacy::invokeFastCallbackInstance1P(
      &callbackProbe, &FastCallbackProbe::onString, stringArgument);
  const bool fastCallbackMatches = natCallbackResult == 7 && stringCallbackResult == 7U &&
                                   callbackProbe.calls == 2 && callbackProbe.lastValue == 7;

  RefProbe oldReference;
  RefProbe newReference;
  RefProbe::sequence = 0;
  RefProbe* reference = &oldReference;
  tmnf::legacy::assignIntrusive(reference, &newReference);
  const bool replacementMatches = reference == &newReference && newReference.refs == 2 &&
                                  oldReference.refs == 0 && newReference.addOrder == 1 &&
                                  oldReference.releaseOrder == 2;
  tmnf::legacy::assignIntrusive(reference, &newReference);
  const bool sameReferenceMatches = RefProbe::sequence == 2;
  tmnf::legacy::assignIntrusive(reference, static_cast<RefProbe*>(nullptr));
  tmnf::legacy::releaseIntrusive(static_cast<RefProbe*>(nullptr));
  const bool releaseMatches = reference == nullptr && newReference.refs == 1 &&
                              newReference.releaseOrder == 3;
  std::array<std::uint32_t, 3> words{10U, 20U, 30U};
  tmnf::legacy::ArrayStorage<std::uint32_t> wordStorage{3U, words.data()};
  std::array<std::uint8_t, 2> bytes{4U, 5U};
  tmnf::legacy::ArrayStorage<std::uint8_t> byteStorage{2U, bytes.data()};
  const bool arrayElementMatches =
      tmnf::legacy::arrayElement(wordStorage, 2U) == &words[2] &&
      tmnf::legacy::arrayElement(byteStorage, 1U) == &bytes[1];
  tmnf::legacy::OriginVectorPair originVector{{1.0F, 2.0F, 3.0F}, {}};
  tmnf::legacy::initializeOriginVector(originVector, {4.0F, 5.0F, 6.0F});
  const bool originVectorMatches =
      originVector.origin == std::array{0.0F, 0.0F, 0.0F} &&
      originVector.vector == std::array{4.0F, 5.0F, 6.0F};

  return empty.data() == nullptr && empty.empty() && narrow == "TrackMania" &&
                 wide == u"Nations" && nullableConstructionMatches &&
                 nullableWideConstructionMatches && legacyUtf8DecoderMatches &&
                 wideConversionMatches && sameBufferWasPreserved &&
                 explicitLengthWasCopied && assigned.empty() &&
                 sameWideBufferWasPreserved && explicitWideLengthWasCopied && hashMatches &&
                 bufferClearMatches && initializedStorageMatches && narrowStorageMatches &&
                 shortNarrowStorageMatches &&
                 sentinelStorageMatches && initializedWideStorageMatches &&
                 wideStorageMatches && longWideStorageMatches &&
                 wideSentinelStorageMatches && contextMatches && lookupMatches &&
                 resolutionMatches && fallbackMatches &&
                 wrapperMatches && writeWrapperMatches && writeUnwindMatches &&
                 readWrapperMatches && argumentWrapperMatches && stopWrapperMatches
                 && pendingArgumentMatches && fullStopMatches && noExtensionStopMatches &&
                 emptyStopMatches && slot94PredicateMatches && fastCallbackMatches &&
                 replacementMatches && sameReferenceMatches && releaseMatches &&
                 arrayElementMatches && originVectorMatches
             ? EXIT_SUCCESS
             : EXIT_FAILURE;
}
