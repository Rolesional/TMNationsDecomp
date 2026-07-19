#pragma once

#include "content/reference_probe.hpp"
#include "tmnf/gameplay/race3_camera.hpp"
#include "tmnf/gameplay/race_start.hpp"

#include <EGL/egl.h>

#include <array>
#include <cstdint>
#include <vector>

struct ANativeWindow;

namespace tmnf::android {

class EglRenderer final {
public:
  EglRenderer() = default;
  ~EglRenderer();

  EglRenderer(const EglRenderer&) = delete;
  EglRenderer& operator=(const EglRenderer&) = delete;

  bool attach(ANativeWindow* window);
  void detach();
  void resize();
  void draw(double seconds);
  void setImage(tmnf::image::RgbaImage image);
  void setHudFont(tmnf::image::RgbaImage image);
  void setScene(tmnf::content::TrackScene scene, tmnf::gbx::SolidModel vehicleSolid,
                tmnf::content::SolidMaterialSet vehicleMaterials);
  void setVehiclePose(tmnf::gbx::Float3 position,
                       std::array<float, 4> orientationWxyz, float frontWheelSteeringRadians,
                       float signedForwardSpeedMetersPerSecond,
                       bool resetCamera = false) noexcept;
  void setRaceHud(float raceTimeSeconds, float speedMetersPerSecond,
                  std::uint32_t checkpointCount,
                  std::uint32_t checkpointTotal, std::uint32_t completedLaps,
                  std::uint32_t lapTotal, bool finished) noexcept;
  void setRaceStartPhase(tmnf::gameplay::RaceStartPhase phase) noexcept;
  void setMapSelector(std::uint32_t mapIndex, std::uint32_t mapTotal,
                      std::int32_t personalBestMilliseconds, std::int32_t authorMilliseconds,
                      bool visible) noexcept;
  void setImageOffset(float x, float y) noexcept;
  [[nodiscard]] bool ready() const noexcept;
  [[nodiscard]] int width() const noexcept { return width_; }
  [[nodiscard]] int height() const noexcept { return height_; }

private:
  struct MeshDraw final {
    int firstIndex = 0;
    int indexCount = 0;
    unsigned int texture = 0;
    bool frontWheel = false;
    tmnf::gbx::Float3 pivot;
  };

  bool createImageResources();
  bool createHudResources();
  bool createMeshResources();
  void drawHud();

  EGLDisplay display_ = EGL_NO_DISPLAY;
  EGLSurface surface_ = EGL_NO_SURFACE;
  EGLContext context_ = EGL_NO_CONTEXT;
  int width_ = 0;
  int height_ = 0;
  tmnf::image::RgbaImage image_;
  unsigned int imageProgram_ = 0;
  unsigned int imageTexture_ = 0;
  unsigned int imageVertexArray_ = 0;
  unsigned int imageVertexBuffer_ = 0;
  int imageScaleUniform_ = -1;
  int imageOffsetUniform_ = -1;
  tmnf::image::RgbaImage hudFont_;
  unsigned int hudTexture_ = 0;
  unsigned int hudVertexArray_ = 0;
  unsigned int hudVertexBuffer_ = 0;
  float hudRaceTimeSeconds_ = 0.0F;
  float hudSpeedMetersPerSecond_ = 0.0F;
  std::uint32_t hudCheckpointCount_ = 0;
  std::uint32_t hudCheckpointTotal_ = 0;
  std::uint32_t hudCompletedLaps_ = 0;
  std::uint32_t hudLapTotal_ = 1;
  bool hudFinished_ = false;
  bool hudVisible_ = false;
  tmnf::gameplay::RaceStartPhase hudRaceStartPhase_ =
      tmnf::gameplay::RaceStartPhase::Hidden;
  std::uint32_t selectorMapIndex_ = 0;
  std::uint32_t selectorMapTotal_ = 0;
  std::int32_t selectorPersonalBestMilliseconds_ = -1;
  std::int32_t selectorAuthorMilliseconds_ = -1;
  bool selectorVisible_ = false;
  tmnf::content::TrackScene scene_;
  tmnf::gbx::SolidModel vehicleSolid_;
  tmnf::content::SolidMaterialSet vehicleMaterials_;
  unsigned int meshProgram_ = 0;
  unsigned int meshVertexArray_ = 0;
  unsigned int meshVertexBuffer_ = 0;
  unsigned int meshIndexBuffer_ = 0;
  unsigned int meshFallbackTexture_ = 0;
  unsigned int vehicleVertexArray_ = 0;
  unsigned int vehicleVertexBuffer_ = 0;
  unsigned int vehicleIndexBuffer_ = 0;
  std::vector<unsigned int> meshMaterialTextures_;
  std::vector<MeshDraw> meshDraws_;
  std::vector<MeshDraw> vehicleDraws_;
  int meshMvpUniform_ = -1;
  int meshTextureUniform_ = -1;
  int meshIndexCount_ = 0;
  int vehicleIndexCount_ = 0;
  tmnf::gbx::Float3 meshWorldCenter_;
  float meshWorldInverseExtent_ = 1.0F;
  tmnf::gbx::Float3 vehiclePosition_;
  std::array<float, 4> vehicleOrientationWxyz_{1.0F, 0.0F, 0.0F, 0.0F};
  float frontWheelSteeringRadians_ = 0.0F;
  float vehicleForwardSpeedMetersPerSecond_ = 0.0F;
  bool vehicleVisible_ = false;
  tmnf::gameplay::Race3Camera camera_;
  bool cameraResetRequested_ = true;
  float imageOffsetX_ = 0.0F;
  float imageOffsetY_ = 0.0F;
};

} // namespace tmnf::android
