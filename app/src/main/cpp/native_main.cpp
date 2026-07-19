#include "content/reference_probe.hpp"
#include "platform/android/audio_output.hpp"
#include "platform/android/egl_renderer.hpp"
#include "platform/android/input_adapter.hpp"
#include "tmnf/content/track_collision.hpp"
#include "tmnf/gameplay/race_start.hpp"
#include "tmnf/gameplay/vehicle.hpp"
#include "tmnf/platform/input.hpp"

#include <android/log.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

namespace {

#if defined(NDEBUG)
constexpr float kVehicleLogIntervalSeconds = 0.5F;
#else
constexpr float kVehicleLogIntervalSeconds = 1.0F / 60.0F;
#endif
constexpr float kDryEngineSpeedInput = -1.0F;

class NativeEngine final {
public:
  explicit NativeEngine(android_app* app) : app_(app), inputAdapter_(input_) {
    app_->userData = this;
    app_->onAppCmd = &NativeEngine::handleCommandThunk;

    std::optional<SavedState> restoredState;
    if (app_->savedState != nullptr && app_->savedStateSize == sizeof(SavedState)) {
      restoredState.emplace();
      std::memcpy(&*restoredState, app_->savedState, sizeof(*restoredState));
    }

    const char* dataPath = app_->activity->internalDataPath != nullptr
                               ? app_->activity->internalDataPath
                               : app_->activity->externalDataPath;
    if (dataPath != nullptr) {
      runtimeDataRoot_ = dataPath;
      __android_log_print(ANDROID_LOG_INFO, "TMNF", "Runtime data root: %s",
                          runtimeDataRoot_.c_str());
      progressPath_ = std::filesystem::path(runtimeDataRoot_) / "nations-progress.txt";
    }
    campaign_ = tmnf::content::loadReferenceCampaign(runtimeDataRoot_.c_str());
    if (!campaign_) {
      __android_log_print(ANDROID_LOG_WARN, "TMNF", "Campaign catalog unavailable: %s",
                          campaign_.error.c_str());
      tmnf::content::ReferenceCampaignMap fallback;
      fallback.groupName = "White";
      fallback.relativePath = "Tracks/Campaigns/Nations/White/A01-Race.Challenge.Gbx";
      fallback.metadata.mapName = "A01-Race";
      campaign_.maps.push_back(std::move(fallback));
      campaign_.error.clear();
    } else {
      __android_log_print(ANDROID_LOG_INFO, "TMNF",
                          "Official Nations campaign ready: %zu ordered maps",
                          campaign_.maps.size());
    }
    loadProgress();
    std::optional<tmnf::gameplay::VehicleState> restoredVehicle;
    std::optional<tmnf::gameplay::RaceStartState> restoredRaceStart;
    if (restoredState.has_value() && restoredState->mapIndex < campaign_.maps.size()) {
      currentMapIndex_ = restoredState->mapIndex;
      restoredVehicle = restoredState->vehicle;
      restoredRaceStart = restoredState->raceStart;
    }
    pendingMapIndex_ = currentMapIndex_;
    if (!loadMap(currentMapIndex_, restoredVehicle, restoredRaceStart)) {
      __android_log_print(ANDROID_LOG_WARN, "TMNF", "Initial campaign map unavailable");
    }
  }

  void run() {
    __android_log_print(ANDROID_LOG_INFO, "TMNF",
                        "Native Android bootstrap entered (target SDK 35)");

    const auto start = std::chrono::steady_clock::now();
    auto lastFrame = start;
    while (app_->destroyRequested == 0) {
      int events = 0;
      android_poll_source* source = nullptr;
      const int timeout = shouldRender() ? 0 : -1;
      const int pollResult =
          ALooper_pollOnce(timeout, nullptr, &events, reinterpret_cast<void**>(&source));
      if (pollResult >= 0 && source != nullptr) {
        source->process(source->app, source);
      }
      if (app_->destroyRequested != 0) {
        return;
      }

      if (android_input_buffer* input = android_app_swap_input_buffers(app_); input != nullptr) {
        inputAdapter_.process(*input, renderer_.width(), renderer_.height());
        android_app_clear_motion_events(input);
        android_app_clear_key_events(input);
      }

      if (resumed_) {
        audio_.update();
      }

      if (shouldRender()) {
        const auto now = std::chrono::steady_clock::now();
        const double deltaSeconds =
            std::clamp(std::chrono::duration<double>(now - lastFrame).count(), 0.0, 0.1);
        lastFrame = now;
        updateSimulation(deltaSeconds);
        const auto elapsed = now - start;
        renderer_.draw(std::chrono::duration<double>(elapsed).count());
      }
    }
  }

private:
  struct SavedState final {
    std::uint32_t mapIndex = 0;
    tmnf::gameplay::VehicleState vehicle;
    tmnf::gameplay::RaceStartState raceStart;
  };

  void loadProgress() {
    if (progressPath_.empty()) {
      return;
    }
    std::ifstream input(progressPath_);
    std::string kind;
    std::string uid;
    while (input >> kind >> uid) {
      if (kind == "selected") {
        const auto found = std::ranges::find_if(
            campaign_.maps, [&uid](const tmnf::content::ReferenceCampaignMap& map) {
              return map.metadata.mapInfo.id == uid;
            });
        if (found != campaign_.maps.end()) {
          currentMapIndex_ = static_cast<std::size_t>(found - campaign_.maps.begin());
        }
      } else if (kind == "record") {
        std::int32_t milliseconds = 0;
        if (!(input >> milliseconds)) {
          break;
        }
        if (milliseconds > 0 && milliseconds <= 359'999'999) {
          records_[uid] = milliseconds;
        }
      } else {
        std::string ignored;
        std::getline(input, ignored);
      }
    }
  }

  void saveProgress() const {
    if (progressPath_.empty() || currentMapIndex_ >= campaign_.maps.size()) {
      return;
    }
    const std::string& selectedUid = campaign_.maps[currentMapIndex_].metadata.mapInfo.id;
    if (selectedUid.empty()) {
      return;
    }
    std::filesystem::path temporary = progressPath_;
    temporary += ".tmp";
    std::ofstream output(temporary, std::ios::trunc);
    output << "selected " << selectedUid << '\n';
    for (const auto& [recordUid, milliseconds] : records_) {
      output << "record " << recordUid << ' ' << milliseconds << '\n';
    }
    output.close();
    if (!output) {
      __android_log_print(ANDROID_LOG_WARN, "TMNF", "Unable to write campaign progress");
      return;
    }
    std::error_code error;
    std::filesystem::rename(temporary, progressPath_, error);
    if (error) {
      __android_log_print(ANDROID_LOG_WARN, "TMNF", "Unable to commit campaign progress: %s",
                          error.message().c_str());
      std::filesystem::remove(temporary, error);
    }
  }

  void updateSelectorHud() {
    if (pendingMapIndex_ >= campaign_.maps.size()) {
      renderer_.setMapSelector(0, 0, -1, -1, false);
      return;
    }
    const tmnf::content::ReferenceCampaignMap& map = campaign_.maps[pendingMapIndex_];
    std::int32_t personalBest = -1;
    if (const auto found = records_.find(map.metadata.mapInfo.id); found != records_.end()) {
      personalBest = found->second;
    }
    renderer_.setMapSelector(static_cast<std::uint32_t>(pendingMapIndex_),
                             static_cast<std::uint32_t>(campaign_.maps.size()), personalBest,
                             map.metadata.medalTimes.authorMilliseconds.value_or(-1),
                             selectorVisible_);
  }

  void moveSelection(const int direction) {
    if (campaign_.maps.empty()) {
      return;
    }
    if (direction < 0) {
      pendingMapIndex_ =
          pendingMapIndex_ == 0U ? campaign_.maps.size() - 1U : pendingMapIndex_ - 1U;
    } else {
      pendingMapIndex_ = (pendingMapIndex_ + 1U) % campaign_.maps.size();
    }
    updateSelectorHud();
  }

  bool loadMap(
      const std::size_t index,
      const std::optional<tmnf::gameplay::VehicleState> restoredVehicle = std::nullopt,
      const std::optional<tmnf::gameplay::RaceStartState> restoredRaceStart = std::nullopt) {
    if (index >= campaign_.maps.size()) {
      return false;
    }
    tmnf::content::ReferenceProbeResult probe = tmnf::content::loadReferenceContent(
        runtimeDataRoot_.c_str(), campaign_.maps[index].relativePath);
    if (!probe) {
      __android_log_print(ANDROID_LOG_WARN, "TMNF", "Campaign map unavailable: %s",
                          probe.error.c_str());
      return false;
    }
    tmnf::content::TrackCollisionBuildResult collision =
        tmnf::content::buildTrackCollision(probe.scene);
    if (!collision) {
      __android_log_print(ANDROID_LOG_ERROR, "TMNF", "Collision construction failed: %s",
                          collision.error.c_str());
      return false;
    }
    collision_ = std::move(collision.collision);
    vehicle_ = {};
    const tmnf::gbx::VehicleTuning* const activeTuning = probe.vehicleTunings.active();
    if (activeTuning == nullptr) {
      __android_log_print(ANDROID_LOG_ERROR, "TMNF", "StadiumCar has no active tuning");
      return false;
    }
    const tmnf::gameplay::VehicleInitializeResult vehicleInitialized =
        vehicle_.initialize(probe.scene, collision_, *activeTuning, &probe.vehicleSolid);
    if (!vehicleInitialized) {
      __android_log_print(ANDROID_LOG_ERROR, "TMNF", "Vehicle initialization failed: %s",
                          vehicleInitialized.error.c_str());
      return false;
    }
    raceStart_.reset();
    if (restoredVehicle.has_value() != restoredRaceStart.has_value() ||
        (restoredRaceStart.has_value() &&
         (!raceStart_.restore(*restoredRaceStart) || !vehicle_.restore(*restoredVehicle)))) {
      raceStart_.reset();
      __android_log_print(ANDROID_LOG_WARN, "TMNF", "Discarded invalid saved race state");
    }

    tmnf::content::ReferenceCampaignMap& selectedMap = campaign_.maps[index];
    if (selectedMap.metadata.mapInfo.id.empty()) {
      selectedMap.metadata.mapInfo = probe.scene.challenge.mapInfo;
      selectedMap.metadata.mapName = probe.scene.challenge.mapName;
      selectedMap.metadata.medalTimes = probe.scene.challenge.medalTimes;
    }
    __android_log_print(ANDROID_LOG_INFO, "TMNF",
                        "Loaded %s (%zu blocks): %zu visible placements, %zu cached models, "
                        "%zu textures, and %dx%d fallback DDS",
                        probe.scene.challenge.mapName.c_str(), probe.scene.challenge.blocks.size(),
                        probe.scene.placements.size(), probe.scene.models.size(),
                        probe.scene.textures.size(), probe.image.width, probe.image.height);
    audio_.setEngine(std::move(probe.soundEngine), std::move(probe.engineSounds));
    audio_.setSound(tmnf::android::RaceSound::Checkpoint, std::move(probe.checkpointSound));
    audio_.setSound(tmnf::android::RaceSound::Finish, std::move(probe.finishSound));
    audio_.setSound(tmnf::android::RaceSound::Countdown1, std::move(probe.countdown1Sound));
    audio_.setSound(tmnf::android::RaceSound::Countdown2, std::move(probe.countdown2Sound));
    audio_.setSound(tmnf::android::RaceSound::Countdown3, std::move(probe.countdown3Sound));
    audio_.setSound(tmnf::android::RaceSound::Go, std::move(probe.goSound));
    renderer_.setImage(std::move(probe.image));
    renderer_.setHudFont(std::move(probe.hudFont));
    renderer_.setScene(std::move(probe.scene), std::move(probe.vehicleSolid),
                       std::move(probe.vehicleMaterials));

    currentMapIndex_ = index;
    pendingMapIndex_ = index;
    selectorVisible_ = false;
    finishAdvanceSeconds_ = 0.0F;
    lastVehicleLogSeconds_ = -1.0F;
    const tmnf::gameplay::VehicleState& vehicle = vehicle_.state();
    audio_.setEngineState(kDryEngineSpeedInput, vehicle.engineRpm, 0.0F);
    renderer_.setVehiclePose(vehicle.position, vehicle.orientationWxyz, 0.0F, 0.0F, true);
    renderer_.setRaceHud(vehicle.raceTimeSeconds, vehicle.speed, vehicle.checkpointCount,
                         static_cast<std::uint32_t>(vehicle_.checkpointTotal()),
                         vehicle.completedLaps, vehicle_.lapTotal(), vehicle.finished);
    renderer_.setRaceStartPhase(raceStart_.current().phase);
    updateSelectorHud();
    saveProgress();

    std::size_t triggerTriangleCount = 0;
    for (const auto& trigger : collision_.triggers) {
      triggerTriangleCount += trigger.indices.size() / 3U;
    }
    __android_log_print(ANDROID_LOG_INFO, "TMNF",
                        "Driving simulation ready: %zu collision triangles, %zu trigger triangles, "
                        "%zu checkpoints x %u laps, spawn=(%.2f, %.2f, %.2f)",
                        collision_.indices.size() / 3U, triggerTriangleCount,
                        vehicle_.checkpointTotal(), vehicle_.lapTotal(), vehicle.position.x,
                        vehicle.position.y, vehicle.position.z);
    return true;
  }

  void recordFinish(const float raceTimeSeconds) {
    if (currentMapIndex_ >= campaign_.maps.size() || !std::isfinite(raceTimeSeconds) ||
        raceTimeSeconds <= 0.0F) {
      return;
    }
    const std::string& uid = campaign_.maps[currentMapIndex_].metadata.mapInfo.id;
    const auto milliseconds = static_cast<std::int32_t>(
        std::clamp(std::lround(raceTimeSeconds * 1000.0F), 1L, 359'999'999L));
    const auto found = records_.find(uid);
    if (found != records_.end() && found->second <= milliseconds) {
      return;
    }
    records_[uid] = milliseconds;
    saveProgress();
    updateSelectorHud();
    __android_log_print(ANDROID_LOG_INFO, "TMNF", "New personal best: %d ms", milliseconds);
  }

  static void handleCommandThunk(android_app* app, const int32_t command) {
    static_cast<NativeEngine*>(app->userData)->handleCommand(command);
  }

  void handleCommand(const int32_t command) {
    switch (command) {
    case APP_CMD_INIT_WINDOW:
      renderer_.attach(app_->window);
      break;
    case APP_CMD_TERM_WINDOW:
      renderer_.detach();
      break;
    case APP_CMD_WINDOW_RESIZED:
    case APP_CMD_CONTENT_RECT_CHANGED:
    case APP_CMD_CONFIG_CHANGED:
      renderer_.resize();
      break;
    case APP_CMD_RESUME:
      resumed_ = true;
      audio_.start();
      break;
    case APP_CMD_PAUSE:
    case APP_CMD_STOP:
      resumed_ = false;
      input_.releaseAll();
      audio_.stop();
      break;
    case APP_CMD_GAINED_FOCUS:
      focused_ = true;
      input_.setFocused(true);
      if (resumed_) {
        audio_.start();
      }
      break;
    case APP_CMD_LOST_FOCUS:
      focused_ = false;
      input_.setFocused(false);
      audio_.stop();
      break;
    case APP_CMD_SAVE_STATE: {
      auto* state = static_cast<SavedState*>(std::malloc(sizeof(SavedState)));
      if (state != nullptr) {
        *state = SavedState{static_cast<std::uint32_t>(currentMapIndex_), vehicle_.state(),
                            raceStart_.state()};
        app_->savedState = state;
        app_->savedStateSize = sizeof(SavedState);
      }
      break;
    }
    default:
      break;
    }
  }

  [[nodiscard]] bool shouldRender() const noexcept {
    return renderer_.ready() && resumed_ && focused_;
  }

  void updateSimulation(const double deltaSeconds) {
    using tmnf::platform::InputAction;
    const tmnf::platform::InputSnapshot input = input_.consume();
    const bool touchReleased = previousTouchActive_ && !input.touchActive;
    previousTouchActive_ = input.touchActive;
    const bool menuButton = input.touchX < -0.78F && input.touchY > 0.72F;
    if (input.wasPressed(InputAction::Menu) || (touchReleased && menuButton)) {
      selectorVisible_ = !selectorVisible_;
      pendingMapIndex_ = currentMapIndex_;
      if (selectorVisible_) {
        audio_.setEngineState(kDryEngineSpeedInput, 0.0F, 0.0F);
      }
      updateSelectorHud();
      return;
    }
    if (selectorVisible_) {
      bool activate = input.wasPressed(InputAction::Confirm);
      if (input.wasPressed(InputAction::SteerLeft)) {
        moveSelection(-1);
      }
      if (input.wasPressed(InputAction::SteerRight)) {
        moveSelection(1);
      }
      if (touchReleased) {
        if (input.touchX < -0.25F) {
          moveSelection(-1);
        } else if (input.touchX > 0.25F) {
          moveSelection(1);
        } else {
          activate = true;
        }
      }
      if (input.wasPressed(InputAction::Cancel)) {
        selectorVisible_ = false;
        pendingMapIndex_ = currentMapIndex_;
        updateSelectorHud();
      } else if (activate) {
        (void)loadMap(pendingMapIndex_);
        updateSelectorHud();
      }
      return;
    }
    if (input.touchActive && menuButton) {
      return;
    }
    if (input.wasPressed(InputAction::Confirm)) {
      audio_.playTone(660.0F, 0.12F);
    }
    if (!vehicle_.initialized()) {
      return;
    }
    bool resetCamera = false;
    if (input.wasPressed(InputAction::Respawn)) {
      vehicle_.reset();
      finishAdvanceSeconds_ = 0.0F;
      resetCamera = true;
      audio_.playTone(440.0F, 0.08F);
    }
    if (vehicle_.state().finished) {
      finishAdvanceSeconds_ += static_cast<float>(deltaSeconds);
      if (finishAdvanceSeconds_ >= 2.0F && currentMapIndex_ + 1U < campaign_.maps.size()) {
        const std::size_t nextMapIndex = currentMapIndex_ + 1U;
        __android_log_print(ANDROID_LOG_INFO, "TMNF", "Advancing to campaign map %zu/%zu",
                            nextMapIndex + 1U, campaign_.maps.size());
        if (loadMap(nextMapIndex)) {
          return;
        }
        finishAdvanceSeconds_ = 0.0F;
      }
    }
    const tmnf::gameplay::RaceStartAdvance raceStart = raceStart_.advance(deltaSeconds);
    renderer_.setRaceStartPhase(raceStart.phase);
    if (raceStart.hasCue(tmnf::gameplay::RaceStartCue::Countdown3)) {
      audio_.playSound(tmnf::android::RaceSound::Countdown3);
    }
    if (raceStart.hasCue(tmnf::gameplay::RaceStartCue::Countdown2)) {
      audio_.playSound(tmnf::android::RaceSound::Countdown2);
    }
    if (raceStart.hasCue(tmnf::gameplay::RaceStartCue::Countdown1)) {
      audio_.playSound(tmnf::android::RaceSound::Countdown1);
    }
    if (raceStart.hasCue(tmnf::gameplay::RaceStartCue::Go)) {
      audio_.playSound(tmnf::android::RaceSound::Go);
    }
    tmnf::gameplay::VehicleControl control;
    control.steering = -input.steering;
    if (std::abs(control.steering) < 0.001F) {
      control.steering = static_cast<float>(input.isHeld(InputAction::SteerLeft)) -
                         static_cast<float>(input.isHeld(InputAction::SteerRight));
    }
    control.throttle =
        std::max(input.throttle, static_cast<float>(input.isHeld(InputAction::Accelerate)));
    control.brake = std::max(input.brake, static_cast<float>(input.isHeld(InputAction::Brake)));
    if (input.touchActive) {
      control.steering = std::clamp(-input.touchX * 1.4F, -1.0F, 1.0F);
      control.throttle = std::clamp((input.touchY - 0.15F) / 0.85F, 0.0F, 1.0F);
      control.brake = std::clamp((-input.touchY - 0.15F) / 0.85F, 0.0F, 1.0F);
    }
    const std::uint32_t previousCheckpointCount = vehicle_.state().checkpointCount;
    const std::uint32_t previousCompletedLaps = vehicle_.state().completedLaps;
    const bool previouslyFinished = vehicle_.state().finished;
    vehicle_.update(static_cast<float>(raceStart.simulationSeconds), control);
    const tmnf::gameplay::VehicleState& vehicle = vehicle_.state();
    const float engineAcceleration = vehicle.speed >= 0.0F ? control.throttle : control.brake;
    audio_.setEngineState(kDryEngineSpeedInput, vehicle.engineRpm, engineAcceleration);
    renderer_.setVehiclePose(vehicle.position, vehicle.orientationWxyz,
                             vehicle_.frontWheelSteeringRadians(), vehicle.speed, resetCamera);
    renderer_.setRaceHud(vehicle.raceTimeSeconds, vehicle.speed, vehicle.checkpointCount,
                         static_cast<std::uint32_t>(vehicle_.checkpointTotal()),
                         vehicle.completedLaps, vehicle_.lapTotal(), vehicle.finished);
    if (vehicle.checkpointCount > previousCheckpointCount) {
      __android_log_print(ANDROID_LOG_INFO, "TMNF", "Checkpoint %u/%zu at %.3f s",
                          vehicle.checkpointCount, vehicle_.checkpointTotal(),
                          vehicle.raceTimeSeconds);
      audio_.playSound(tmnf::android::RaceSound::Checkpoint);
    }
    if (vehicle.completedLaps > previousCompletedLaps) {
      __android_log_print(ANDROID_LOG_INFO, "TMNF", "Lap %u/%u at %.3f s", vehicle.completedLaps,
                          vehicle_.lapTotal(), vehicle.raceTimeSeconds);
      audio_.playTone(940.0F, 0.12F);
    }
    if (vehicle.finished && !previouslyFinished) {
      finishAdvanceSeconds_ = 0.0F;
      __android_log_print(ANDROID_LOG_INFO, "TMNF", "Race finished at %.3f s",
                          vehicle.raceTimeSeconds);
      recordFinish(vehicle.raceTimeSeconds);
      audio_.playSound(tmnf::android::RaceSound::Finish);
    }
    if ((control.throttle > 0.01F || control.brake > 0.01F || std::abs(control.steering) > 0.01F) &&
        vehicle.elapsedSeconds - lastVehicleLogSeconds_ >= kVehicleLogIntervalSeconds) {
      lastVehicleLogSeconds_ = vehicle.elapsedSeconds;
      __android_log_print(ANDROID_LOG_DEBUG, "TMNF",
                          "Vehicle pose: position=(%.2f, %.2f, %.2f), speed=%.2f, heading=%.3f, "
                          "grounded=%d",
                          vehicle.position.x, vehicle.position.y, vehicle.position.z, vehicle.speed,
                          vehicle.heading, vehicle.grounded ? 1 : 0);
    }
  }

  android_app* app_;
  std::string runtimeDataRoot_;
  tmnf::platform::InputState input_;
  tmnf::android::InputAdapter inputAdapter_;
  tmnf::android::AudioOutput audio_;
  tmnf::android::EglRenderer renderer_;
  tmnf::content::ReferenceCampaignResult campaign_;
  tmnf::content::TrackCollision collision_;
  tmnf::gameplay::RaceStartClock raceStart_;
  tmnf::gameplay::VehicleSimulation vehicle_;
  std::filesystem::path progressPath_;
  std::unordered_map<std::string, std::int32_t> records_;
  std::size_t currentMapIndex_ = 0;
  std::size_t pendingMapIndex_ = 0;
  bool resumed_ = false;
  bool focused_ = false;
  bool selectorVisible_ = false;
  bool previousTouchActive_ = false;
  float finishAdvanceSeconds_ = 0.0F;
  float lastVehicleLogSeconds_ = -1.0F;
};

} // namespace

extern "C" __attribute__((visibility("default"))) void android_main(android_app* app) {
  NativeEngine engine(app);
  engine.run();
}
